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
