#include "boot/loaders/Multiboot2.hpp"
#include "kernel/memory/physical/MemoryManager.hpp"
#include "keyboard.hpp"
#include "runtime/mem.hpp"
#include "vga.hpp"
#include <stddef.h>
#include <stdint.h>

// ---- лог-помощники ----
namespace log {

inline void write(const char *s) { vga::write(s); }
inline void nl() { vga::write("\n"); }

inline char hex_digit(unsigned v) { return v < 10 ? '0' + v : 'A' + (v - 10); }

inline void write_hex8(uint8_t x) {
  char buf[4] = {'0', 'x', 0, 0};
  buf[2] = hex_digit((x >> 4) & 0xF);
  buf[3] = hex_digit(x & 0xF);
  vga::write(buf);
}

inline void write_hex32(uint32_t x) {
  char buf[11];
  buf[0] = '0';
  buf[1] = 'x';
  for (int i = 0; i < 8; i++) {
    buf[9 - i] = hex_digit(x & 0xF);
    x >>= 4;
  }
  buf[10] = '\0';
  vga::write(buf);
}

inline void write_hex64(uint64_t x) {
  char buf[19];
  buf[0] = '0';
  buf[1] = 'x';
  for (int i = 0; i < 16; i++) {
    unsigned v = (x >> ((15 - i) * 4)) & 0xF;
    buf[2 + i] = v < 10 ? char('0' + v) : char('A' + (v - 10));
  }
  buf[18] = '\0'; // фикс нуль-терминатора
  vga::write(buf);
}

inline void write_dec(uint64_t x) {
  char buf[32];
  int i = 31;
  buf[i] = '\0';
  if (x == 0) {
    vga::write("0");
    return;
  }
  while (x && i > 0) {
    buf[--i] = char('0' + (x % 10));
    x /= 10;
  }
  vga::write(&buf[i]);
}

// безопасная печать C-строки с ограничением длины
inline void write_cstr_bounded(const char *s, size_t max_bytes) {
  if (!s) {
    vga::write("(null)");
    return;
  }
  for (size_t i = 0; i < max_bytes; ++i) {
    if (s[i] == '\0')
      return vga::write(s); // обычный путь
  }
  // если по какой-то причине нет '\0' в пределах max_bytes
  for (size_t i = 0; i < max_bytes; ++i) {
    char c[2] = {s[i], 0};
    vga::write(c);
  }
}

} // namespace log

// Linker-provided kernel bounds (define in linker.ld)
extern "C" char _kernel_start, _kernel_end;

extern "C" {
// simple portable unsigned 64-bit divide/mod helpers
static inline uint64_t udiv64(uint64_t n, uint64_t d, uint64_t *rem_out) {
  if (d == 0) {
    if (rem_out)
      *rem_out = 0;
    return ~uint64_t(0);
  }
  uint64_t q = 0, r = 0;
  for (int i = 63; i >= 0; --i) {
    r = (r << 1) | ((n >> i) & 1);
    if (r >= d) {
      r -= d;
      q |= (uint64_t(1) << i);
    }
  }
  if (rem_out)
    *rem_out = r;
  return q;
}

// GCC/Clang expect these exact unmangled names
uint64_t __udivdi3(uint64_t n, uint64_t d) {
  uint64_t r;
  return udiv64(n, d, &r);
}

uint64_t __umoddi3(uint64_t n, uint64_t d) {
  uint64_t r;
  (void)udiv64(n, d, &r);
  return r;
}
}
using namespace kernel_name::memory::physical::multiboot2;

struct Range {
  uint64_t lo, hi;
  char tag;
};
static inline bool in_range(uint64_t a, const Range &r) {
  return a >= r.lo && a < r.hi;
}

static inline uint32_t mmap_type_at(const Info *info, uint64_t pos) {
  const TagMMap *mm = nullptr;
  for (const Tag &t : *info) {
    if (t.type == TYPE::MB2_TAG_MMAP) {
      mm = (const TagMMap *)&t;
      break;
    }
  }
  if (!mm)
    return 2;
  for (const MMapEntry &e : *mm) {
    if (pos >= e.addr && pos < e.addr + e.len)
      return e.type;
  }
  return 2;
}

static inline uint64_t fb_size_bytes(const TagFramebuffer *fb) {
  return fb ? (uint64_t)fb->pitch * fb->height : 0;
}

extern "C" char _kernel_start, _kernel_end;
static unsigned collect_specials(const Info *info, Range *out, unsigned cap) {
  unsigned n = 0;
  if (n < cap)
    out[n++] = Range{(uint64_t)(uintptr_t)&_kernel_start,
                     (uint64_t)(uintptr_t)&_kernel_end, 'K'};

  if (n < cap) {
    const uint64_t mb2_lo = (uint64_t)(uintptr_t)info;
    const uint64_t mb2_hi = mb2_lo + info->total_size;
    out[n++] = Range{mb2_lo, mb2_hi, 'B'};
  }

  const TagFramebuffer *fb_tag = nullptr;
  for (const Tag &t : *info) {
    if (t.type == TYPE::MB2_TAG_END)
      break;
    if (t.type == TYPE::MB2_TAG_MODULE) {
      const auto *m = (const TagModule *)&t;
      if (n < cap)
        out[n++] = Range{m->mod_start, m->mod_end, 'M'};
    } else if (t.type == TYPE::MB2_TAG_FRAMEBUFFER) {
      fb_tag = (const TagFramebuffer *)&t;
    }
  }
  if (fb_tag && fb_tag->addr) {
    const uint64_t fbl = fb_tag->addr;
    const uint64_t fbh = fbl + fb_size_bytes(fb_tag);
    if (fbh > fbl && n < cap)
      out[n++] = Range{fbl, fbh, 'F'};
  }
  return n;
}

// Fills out[0..cols-1] with classes and overlays specials; no per-column 64-bit
// div.
static void render_slice_bar(const Info *info, uint64_t lo, uint64_t step,
                             unsigned cols, const Range *sp, unsigned spn,
                             char *out /*>=cols+1*/) {
  uint64_t pos = lo;
  const uint64_t q = step / cols;
  const uint64_t r = step % cols;
  uint64_t err = 0;

  for (unsigned i = 0; i < cols; ++i) {
    char ch;
    const uint32_t ty = mmap_type_at(info, pos);
    switch (ty) {
    case 1:
      ch = 'U';
      break; // usable
    case 2:
      ch = 'R';
      break; // reserved
    case 3:
      ch = 'A';
      break; // acpi reclaim
    case 4:
      ch = 'N';
      break; // acpi nvs
    case 5:
      ch = 'X';
      break; // bad
    default:
      ch = '?';
      break; // other/unknown
    }
    // overlay specials if any
    for (unsigned j = 0; j < spn; ++j) {
      if (in_range(pos, sp[j])) {
        ch = sp[j].tag;
        break;
      }
    }
    out[i] = ch;

    pos += q;
    err += r;
    if (err >= cols) {
      pos += 1;
      err -= cols;
    }
  }
  out[cols] = '\0';
}

static void animate_mem_bar_basic(const Info *info) {
  constexpr uint64_t MiB = 1024ull * 1024;
  constexpr uint64_t GiB = 1024ull * MiB;
  const uint64_t total = 16 * GiB;
  const uint64_t step = 64 * MiB;

  // Reserve a line for the bar: print legend ONCE, then remember Y.
  vga::write("Legend: U=Usable R=Reserved A=ACPIrec N=ACPINVS X=Bad K=Kernel "
             "B=MB2 M=Module F=FB\n");

  const uint16_t W = vga::width();
  const unsigned cols =
      (W > 20 ? W - 20 : (W ? W - 1 : 1)); // leave room for header
  if (cols < 8)
    return;

  // capture the y line to reuse
  const uint16_t bar_y = vga::y();
  // ensure we have a clean line
  vga::move_cursor(0, bar_y);
  for (uint16_t i = 0; i < W; ++i)
    vga::write(" ");

  Range specials[64];
  const unsigned spn = collect_specials(info, specials, 64);

  // reusable buffers
  // header ~ 20 chars, bar = cols, plus pad
  char bar[512]; // enough for typical VGA widths
  char header[64];

  for (uint64_t lo = 0; lo < total; lo += step) {
    const uint64_t hi = lo + step;

    // construct header: "Phys 0xXXXXXXXXXXXXXXX..0xYYYYYYYYYYYYYYYY : "
    // keep it stable length (pad later)
    vga::move_cursor(0, bar_y);

    // write header in-place
    vga::write("Phys ");
    log::write_hex64(lo);
    vga::write(" .. ");
    log::write_hex64(hi);
    vga::write(" : ");

    render_slice_bar(info, lo, step, cols, specials, spn, bar);
    vga::write(bar);

    // pad to erase leftovers up to full width, then return carriage
    uint16_t printed =
        6 /*Phys ␣*/ + 2 /* .. */ + 3 /* ␣: ␣ */ + 16 + 16 /*hexes*/ + cols;
    if (printed < W) {
      for (uint16_t i = 0; i < (W - printed); ++i)
        vga::write(" ");
    }
    vga::write("\r"); // back to column 0 on same bar_y

    // tiny delay; replace with a proper timer later
    for (volatile int i = 0; i < 400000; ++i) {
      __asm__ __volatile__("pause");
    }
  }

  // move cursor to next line once done (optional)
  vga::move_cursor(0, (uint16_t)(bar_y + 1));
}

// имя типа тега
static const char *
tag_type_name(kernel_name::memory::physical::multiboot2::TYPE t) {
  using T = kernel_name::memory::physical::multiboot2::TYPE;
  switch (t) {
  case T::MB2_TAG_END:
    return "END";
  case T::MB2_TAG_CMDLINE:
    return "CMDLINE";
  case T::MB2_TAG_BOOTLOADER:
    return "BOOTLOADER";
  case T::MB2_TAG_MODULE:
    return "MODULE";
  case T::MB2_TAG_BASIC_MEM:
    return "BASIC_MEM";
  case T::MB2_TAG_BOOTDEV:
    return "BOOTDEV";
  case T::MB2_TAG_MMAP:
    return "MMAP";
  case T::MB2_TAG_ACPI_OLD:
    return "ACPI_OLD";
  case T::MB2_TAG_ACPI_NEW:
    return "ACPI_NEW";
  case T::MB2_TAG_FRAMEBUFFER:
    return "FRAMEBUFFER";
  default:
    return "UNKNOWN";
  }
}

extern "C" void kmain(uint32_t magic, uint32_t mb_info_phys) {
  using namespace kernel_name::memory::physical::multiboot2;

  constexpr uint32_t MB2_MAGIC = 0x36D76289;
  if (magic != MB2_MAGIC) {
    // vga::write("Not multiboot2!\n");
    for (;;)
      __asm__ __volatile__("hlt");
  }

  const auto *info = reinterpret_cast<const Info *>(mb_info_phys);
  vga::init(0x07, info);

  vga::write("Multiboot2 info @ ");
  log::write_hex32(mb_info_phys);
  vga::write(" total_size=");
  log::write_dec(info->total_size);
  log::nl();
  log::nl();

  uint64_t total_available_bytes = 0;

  for (const Tag &tag : *info) {
    if (tag.type == TYPE::MB2_TAG_END) {
      vga::write("[END] reached.\n");
      break;
    }

    vga::write("Tag ");
    vga::write(tag_type_name(tag.type));
    vga::write(" (type=");
    log::write_dec(static_cast<uint32_t>(tag.type));
    vga::write(") size=");
    log::write_dec(tag.size);
    vga::write(" at=");
    log::write_hex32(reinterpret_cast<uint32_t>(&tag));
    log::nl();

    switch (tag.type) {
    case TYPE::MB2_TAG_CMDLINE: {
      const auto *t = reinterpret_cast<const TagString *>(&tag);
      vga::write("  cmdline: \"");
      // полезная нагрузка = size - sizeof(TagString)
      log::write_cstr_bounded(t->c_str(), t->size - sizeof(TagString));
      vga::write("\"\n");
      break;
    }
    case TYPE::MB2_TAG_BOOTLOADER: {
      const auto *t = reinterpret_cast<const TagString *>(&tag);
      vga::write("  bootloader: \"");
      log::write_cstr_bounded(t->c_str(), t->size - sizeof(TagString));
      vga::write("\"\n");
      break;
    }
    case TYPE::MB2_TAG_MODULE: {
      const auto *t = reinterpret_cast<const TagModule *>(&tag);
      vga::write("  module range: [");
      log::write_hex32(t->mod_start);
      vga::write(", ");
      log::write_hex32(t->mod_end);
      vga::write(") bytes=");
      log::write_dec(t->bytes());
      log::nl();
      vga::write("  module string: \"");
      log::write_cstr_bounded(t->c_str(), t->size - sizeof(TagModule));
      vga::write("\"\n");
      break;
    }
    case TYPE::MB2_TAG_BASIC_MEM: {
      const auto *t = reinterpret_cast<const TagBasicMem *>(&tag);
      vga::write("  lower_kb=");
      log::write_dec(t->mem_lower_kb);
      vga::write(" upper_kb=");
      log::write_dec(t->mem_upper_kb);
      log::nl();
      break;
    }
    case TYPE::MB2_TAG_BOOTDEV: {
      const auto *t = reinterpret_cast<const TagBootDev *>(&tag);
      vga::write("  biosdev=");
      log::write_hex8(static_cast<uint8_t>(t->biosdev & 0xFF));
      vga::write(" slice=");
      log::write_dec(t->slice);
      vga::write(" part=");
      log::write_dec(t->part);
      log::nl();
      break;
    }
    case TYPE::MB2_TAG_ACPI_OLD: {
      const auto *t = reinterpret_cast<const TagAcpiOld *>(&tag);
      vga::write("  RSDP (v1) @ ");
      log::write_hex32(t->rsdp);
      log::nl();
      break;
    }
    case TYPE::MB2_TAG_ACPI_NEW: {
      const auto *t = reinterpret_cast<const TagAcpiNew *>(&tag);
      vga::write("  RSDP (v2+) @ ");
      log::write_hex64(t->rsdp);
      log::nl();
      break;
    }
    case TYPE::MB2_TAG_MMAP: {
      const auto *mm = reinterpret_cast<const TagMMap *>(&tag);
      const uint32_t payload =
          (mm->size > sizeof(TagMMap)) ? (mm->size - sizeof(TagMMap)) : 0;
      const uint32_t count = (mm->entry_size ? (payload / mm->entry_size) : 0);

      vga::write("  MMap: entry_size=");
      log::write_dec(mm->entry_size);
      vga::write(" entry_ver=");
      log::write_dec(mm->entry_version);
      vga::write(" entries=");
      log::write_dec(count);
      if (mm->entry_size != sizeof(MMapEntry)) {
        vga::write(" (warn: stride != sizeof(MMapEntry)=");
        log::write_dec(sizeof(MMapEntry));
        vga::write(")");
      }
      log::nl();

      uint32_t idx = 0;
      for (const MMapEntry &e : *mm) {
        vga::write("    [");
        log::write_dec(idx++);
        vga::write("] addr=");
        log::write_hex64(e.addr);
        vga::write(" len=");
        log::write_dec(e.len);
        vga::write(" type=");
        log::write_dec(e.type);
        // Типы по соглашению:
        // 1=available, 2=reserved, 3=ACPI reclaim, 4=ACPI NVS, 5=bad
        vga::write(" (");
        switch (e.type) {
        case 1:
          vga::write("available");
          break;
        case 2:
          vga::write("reserved");
          break;
        case 3:
          vga::write("acpi-reclaim");
          break;
        case 4:
          vga::write("acpi-nvs");
          break;
        case 5:
          vga::write("bad");
          break;
        default:
          vga::write("other");
          break;
        }
        vga::write(")\n");

        if (e.type == 1)
          total_available_bytes += e.len;
      }

      vga::write("  MMap total available=");
      log::write_dec(total_available_bytes);
      vga::write(" bytes (");
      // грубая печать в MiB
      log::write_dec(total_available_bytes >> 20);
      vga::write(" MiB)\n");
      break;
    }
    case TYPE::MB2_TAG_FRAMEBUFFER: {
      vga::write("  (no parser for this tag yet)\n");
      break;
    }
    default:
      // Неподдержанный из вашего перечисления
      vga::write("  (no parser for this tag yet)\n");
      break;
    }

    log::nl();
  }

  vga::write("Summary: total available RAM = ");
  log::write_dec(total_available_bytes);
  vga::write(" bytes (");
  log::write_dec(total_available_bytes >> 20);
  vga::write(" MiB)\n");

  kernel_name::memory::physical::Manager::instance().fromMB2(mb_info_phys);
}
