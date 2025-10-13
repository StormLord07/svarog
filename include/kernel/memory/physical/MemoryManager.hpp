#pragma once
#include "boot/loaders/Multiboot2.hpp"
#include <cstddef>
#include <cstdint>

namespace kernel_name::memory::physical {

enum MMIO_FLAGS : uint32_t {
  MMIO_READ = 1 << 0,
  MMIO_WRITE = 1 << 1,
  MMIO_PREFETCH = 1 << 2,
  MMIO_CACHEABLE = 1 << 3,
  MMIO_MMCONFIG = 1 << 4, // PCIe config space window
};

// Generic MMIO struct
struct MMIO {
  // std::string name; // We'd need to load libc++ for that
  std::uintptr_t start;
  std::size_t size;
  // std::string type; // We'd need to load libc++ for that, that is a
  // enforcement policy
  uint32_t flags;
};

enum class TYPE : uint32_t {
  FREE = 1,
  RESERVER = 2,
};

struct Partition {
  TYPE type{};
  // The next part should be either 32 or 64, for simplicity we make it 64 for
  // now
  uint64_t start_address;
  uint64_t size;
};

class Manager {
public:
  static Manager &instance() {
    static Manager mgr;
    return mgr;
  }
  struct Entry { // temp solution
    Entry *next;
    uint64_t base;
    uint64_t ptr;
    uint64_t size;
  };

  struct Framebuffer {
    uint32_t width;
    uint32_t height;
  };
  // Build from Multiboot2 info. No dynamic allocation used.
  void fromMB2(uint32_t mb_info_phys) {

    framebuffer_ = {};
    count_ = 0;
    head_ = nullptr;

    const auto *info = reinterpret_cast<const multiboot2::Info *>(
        static_cast<std::uintptr_t>(mb_info_phys));
    if (!info)
      return;

    // 1) scan once: grab framebuffer + collect available mmap entries into
    // arena_
    for (const multiboot2::Tag &tag : *info) {
      if (tag.type == multiboot2::TYPE::MB2_TAG_END)
        break;

      if (tag.type == multiboot2::TYPE::MB2_TAG_FRAMEBUFFER) {
        const auto &fb =
            *reinterpret_cast<const multiboot2::TagFramebuffer *>(&tag);
        framebuffer_.width = fb.width;
        framebuffer_.height = fb.height;
      } else if (tag.type == multiboot2::TYPE::MB2_TAG_MMAP) {
        const auto &mm = *reinterpret_cast<const multiboot2::TagMMap *>(&tag);
        for (const multiboot2::MMapEntry &e : mm) {
          if (e.type != 1 || e.len == 0)
            continue; // 1 = available
          if (count_ >= MaxEntries)
            continue; // out of arena; drop extra
          Entry &slot = arena_[count_++];
          slot.next = nullptr;
          slot.base = static_cast<uint64_t>(e.addr);
          slot.ptr = static_cast<uint64_t>(e.addr);
          slot.size = static_cast<uint64_t>(e.len);
        }
      }
    }

    if (count_ == 0) {
      head_ = nullptr;
      return;
    }

    // 2) sort by base (stable insertion sort; small N, no deps)
    for (std::size_t i = 1; i < count_; ++i) {
      Entry key = arena_[i];
      std::size_t j = i;
      while (j > 0 && arena_[j - 1].base > key.base) {
        arena_[j] = arena_[j - 1];
        --j;
      }
      arena_[j] = key;
    }

    // 3) (optional) coalesce adjacent segments
    std::size_t w = 0;
    for (std::size_t i = 0; i < count_; ++i) {
      const uint64_t cur_b = arena_[i].base;
      const uint64_t cur_e = cur_b + arena_[i].size;

      if (w == 0) {
        arena_[w++] = arena_[i];
      } else {
        uint64_t prev_b = arena_[w - 1].base;
        uint64_t prev_e = prev_b + arena_[w - 1].size;
        if (cur_b == prev_e) {
          // merge
          arena_[w - 1].size += arena_[i].size;
        } else {
          arena_[w++] = arena_[i];
        }
      }
    }
    count_ = w;

    // 4) link them
    for (std::size_t i = 0; i + 1 < count_; ++i)
      arena_[i].next = &arena_[i + 1];
    arena_[count_ - 1].next = nullptr;
    head_ = &arena_[0];
  }

  // --- Аллокация/освобождение ---
  void *alloc(std::size_t n) {
    // выравнивание до 16
    n = (n + 15) & ~std::size_t(15);

    if (!head_)
      return nullptr;
    // простой bump в первом регионе
    if (head_->ptr + n > head_->base + head_->size) {
      return nullptr; // нет места
    }
    void *p = reinterpret_cast<void *>(head_->ptr);
    head_->ptr += n;
    return p;
  }

  void free(void *) {
    // no-op пока
  }
  // Accessors
  const Framebuffer &framebuffer() const { return framebuffer_; }
  const Entry *free_list() const { return head_; }
  std::size_t count() const { return count_; }

private:
  // adjust as needed; 128 entries usually plenty for early boot
  static constexpr std::size_t MaxEntries = 128;

  Framebuffer framebuffer_{};
  Entry *head_ = nullptr;
  std::size_t count_ = 0;

  // internal arena holding all nodes (no dynamic allocation)
  Entry arena_[MaxEntries]{};
};

} // namespace kernel_name::memory::physical
