#pragma once
#include <cstddef>
#include <cstdint>

namespace kernel_name::memory::physical::multiboot2 {

enum class TYPE : uint32_t {
  MB2_TAG_END = 0,
  MB2_TAG_CMDLINE = 1,
  MB2_TAG_BOOTLOADER = 2,
  MB2_TAG_MODULE = 3,
  MB2_TAG_BASIC_MEM = 4,
  MB2_TAG_BOOTDEV = 5,
  MB2_TAG_MMAP = 6,
  MB2_TAG_FRAMEBUFFER = 8,
  MB2_TAG_ACPI_OLD = 14,
  MB2_TAG_ACPI_NEW = 15,
};

// ---- Generic tag header (for every tag) ----
struct alignas(8) Tag {
  TYPE type;     // one of MB2_TAG_*
  uint32_t size; // bytes, includes this header
  // followed by payload, then padding to 8-byte alignment
  const Tag *next() const;
};

// ---- Top-level info header ----
struct alignas(8) Info {
  uint32_t total_size;
  uint32_t reserved;

  struct Iter {
    const Tag *cur;
    const Tag *limit; // byte-end sentinel
    const Tag &operator*() const { return *cur; }
    const Tag *operator->() const { return cur; }
    Iter &operator++() {
      cur = cur->next();
      return *this;
    }
    bool operator!=(const Iter &o) const { return cur != o.cur; }
  };

  Iter begin() const {
    auto *first = reinterpret_cast<const Tag *>(
        reinterpret_cast<const std::byte *>(this) + sizeof(Info));
    auto *lim = reinterpret_cast<const Tag *>(
        reinterpret_cast<const std::byte *>(this) + total_size);
    return Iter{first, lim};
  }
  Iter end() const {
    auto *lim = reinterpret_cast<const Tag *>(
        reinterpret_cast<const std::byte *>(this) + total_size);
    return Iter{lim, lim};
  }
};

// -----------------------------------------------------------
// TAG 1 / TAG 2: Строковые теги (cmdline, bootloader name)
// Полезная нагрузка — C-строка с завершающим '\0'.
// -----------------------------------------------------------
struct alignas(8) TagString {
  TYPE type;     // == MB2_TAG_CMDLINE или MB2_TAG_BOOTLOADER
  uint32_t size; // полный размер
  // далее — нуль-терминированная строка

  const char *c_str() const {
    return reinterpret_cast<const char *>(this) + sizeof(TagString);
  }

  // длина строки без учёта завершающего '\0' (если хочется)
  std::size_t length() const {
    const auto total = size - sizeof(TagString);
    // строка гарантированно нуль-терминирована по спецификации
    // возвращаем «до первого '\0'» в пределах total
    const char *s = c_str();
    std::size_t n = 0;
    while (n < total && s[n] != '\0')
      ++n;
    return n;
  }
};

// -----------------------------------------------------------
// TAG 3: Module
// Полезные поля + нуль-терминированная строка (имя/args).
// -----------------------------------------------------------
struct alignas(8) TagModule {
  TYPE type;     // == MB2_TAG_MODULE
  uint32_t size; // полный размер
  uint32_t mod_start;
  uint32_t mod_end;
  // далее — нуль-терминированная строка (имя/аргументы)

  const char *c_str() const {
    return reinterpret_cast<const char *>(this) + sizeof(TagModule);
  }

  std::size_t string_length() const {
    const auto hdr = sizeof(TagModule);
    if (size <= hdr)
      return 0;
    const auto total = size - hdr;
    const char *s = c_str();
    std::size_t n = 0;
    while (n < total && s[n] != '\0')
      ++n;
    return n;
  }

  std::size_t bytes() const {
    return mod_end >= mod_start ? (mod_end - mod_start) : 0;
  }
};

// -----------------------------------------------------------
// TAG 4: Basic mem info (нижняя/верхняя память в КБ, «как в MB1»)
// -----------------------------------------------------------
struct alignas(8) TagBasicMem {
  TYPE type;             // == MB2_TAG_BASIC_MEM
  uint32_t size;         // == sizeof(TagBasicMem)
  uint32_t mem_lower_kb; // обычно ~640
  uint32_t mem_upper_kb; // начиная от 1МБ
};

// -----------------------------------------------------------
// TAG 5: Boot device (BIOS drive + разделы)
// (исторически biosdev/partition/subpartition).
// -----------------------------------------------------------
struct alignas(8) TagBootDev {
  TYPE type;        // == MB2_TAG_BOOTDEV
  uint32_t size;    // == sizeof(TagBootDev)
  uint32_t biosdev; // BIOS drive number
  uint32_t slice;   // первичный раздел
  uint32_t part;    // под-раздел (или 0xFFFFFFFF если нет)
};

// ---- Memory map entry layout (current spec) ----
struct MMapEntry {
  uint64_t addr; // physical base address
  uint64_t len;  // length in bytes
  uint32_t type; // 1=available, 2=reserved, 3=ACPI reclaim, 4=ACPI NVS, 5=bad
  uint32_t zero; // reserved
};

// ---- Memory map tag header (type = 6) ----
struct alignas(8) TagMMap {
  TYPE type;                // == TYPE::MB2_TAG_MMAP
  std::uint32_t size;       // header + entries
  std::uint32_t entry_size; // stride for iteration
  std::uint32_t entry_version;

  struct Iter {
    const std::byte *cur; // byte pointer inside entries blob
    const std::byte *end; // byte end (this + size)
    std::uint32_t stride; // == entry_size
    const MMapEntry &operator*() const {
      return *reinterpret_cast<const MMapEntry *>(cur);
    }
    const MMapEntry *operator->() const {
      return reinterpret_cast<const MMapEntry *>(cur);
    }
    Iter &operator++() {
      cur += stride; // The data of entry size is stored in the entry_size var,
                     // not always sizeof(MMapEntry)?
      return *this;
    }
    bool operator!=(const Iter &o) const { return cur != o.cur; }
  };

  Iter begin() const {
    const auto *start =
        reinterpret_cast<const std::byte *>(this) + sizeof(TagMMap);
    const auto *stop = reinterpret_cast<const std::byte *>(this) + size;
    return Iter{start, stop, entry_size};
  }
  Iter end() const {
    const auto *stop = reinterpret_cast<const std::byte *>(this) + size;
    return Iter{stop, stop, entry_size};
  }
};

enum class FBType : uint8_t { Indexed = 0, RGB = 1, EGAText = 2 };

struct alignas(8) TagFramebuffer {
  TYPE type; // == Framebuffer
  uint32_t size;
  uint64_t addr;   // phys address (unused here)
  uint32_t pitch;  // bytes per scanline
  uint32_t width;  // pixels (or chars if EGAText)
  uint32_t height; // pixels (or chars if EGAText)
  uint8_t bpp;
  FBType fbtype;
  uint8_t reserved;
  // trailing color info ignored
};
// -----------------------------------------------------------
// TAG 14: ACPI (RSDP) — «старый» (v1). Обычно 32-битный указатель.
// -----------------------------------------------------------
struct alignas(8) TagAcpiOld {
  TYPE type;     // == MB2_TAG_ACPI_OLD
  uint32_t size; // == sizeof(TagAcpiOld)
  uint32_t rsdp; // физический адрес RSDP (RSDP 1.0)
  uint32_t _pad; // для 8-байтного выравнивания
};

// -----------------------------------------------------------
// TAG 15: ACPI (RSDP) — «новый» (v2, ACPIs 2+). 64-битный указатель.
// -----------------------------------------------------------
struct alignas(8) TagAcpiNew {
  TYPE type;     // == MB2_TAG_ACPI_NEW
  uint32_t size; // == sizeof(TagAcpiNew)
  uint64_t rsdp; // физический адрес RSDP (RSDP 2.0+)
};

} // namespace kernel_name::memory::physical::multiboot2