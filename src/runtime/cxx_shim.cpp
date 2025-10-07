#include "runtime/cxx_shim.hpp"
#include "kernel/memory/physical/MemoryManager.hpp" // Manager

namespace {
// --- Bootstrap bump heap for very early allocations ---
alignas(16) static std::uint8_t BOOT_HEAP[256 * 1024]; // 256 KiB
static size_t bs_off = 0;

void *bs_alloc(size_t n) {
  n = (n + 15) & ~size_t(15);
  if (bs_off + n > sizeof(BOOT_HEAP))
    return nullptr;
  void *p = BOOT_HEAP + bs_off;
  bs_off += n;
  return p;
}
bool in_bootstrap(const void *p) {
  auto base = reinterpret_cast<std::uintptr_t>(BOOT_HEAP);
  auto end = base + sizeof(BOOT_HEAP);
  auto q = reinterpret_cast<std::uintptr_t>(p);
  return q >= base && q < end;
}

// Header for aligned_alloc so free() can find the real base
struct AlignedHdr {
  std::uint32_t magic;
  void *base;
};
constexpr std::uint32_t ALIGNED_MAGIC = 0xC0A1A1EDu;
} // namespace

// ===== Global allocation API
// ==================================================
extern "C" void *kmalloc(size_t n) {
  // Prefer real manager (after fromMB2). Falls back to bootstrap.
  auto &mgr = kernel_name::memory::physical::Manager::instance();
  if (void *p = mgr.alloc(n))
    return p;
  return bs_alloc(n);
}

extern "C" void kfree(void *p) {
  if (!p)
    return;
  if (in_bootstrap(p))
    return; // bootstrap blocks leak by design
  // Manager::free is a no-op for now, keep the call for future implementation:
  // kernel_name::memory::physical::Manager::instance().free(p);
  (void)p;
}

// ===== libc layer expected by libc++/abi =====================================
extern "C" {

void *malloc(size_t n) { return kmalloc(n); }

void free(void *p) {
  if (!p)
    return;
  // detect aligned_alloc blocks
  auto *hdr = reinterpret_cast<AlignedHdr *>(
      reinterpret_cast<unsigned char *>(p) - sizeof(AlignedHdr));
  if (hdr->magic == ALIGNED_MAGIC && hdr->base) {
    kfree(hdr->base);
  } else {
    kfree(p);
  }
}

void *aligned_alloc(size_t align, size_t size) {
  if (align < sizeof(void *) || (align & (align - 1)) != 0)
    return kmalloc(size);
  size_t total = size + align - 1 + sizeof(AlignedHdr);
  auto raw = reinterpret_cast<std::uintptr_t>(kmalloc(total));
  if (!raw)
    return nullptr;

  std::uintptr_t after_hdr = raw + sizeof(AlignedHdr);
  std::uintptr_t aligned =
      (after_hdr + (align - 1)) & ~(std::uintptr_t)(align - 1);

  auto *hdr = reinterpret_cast<AlignedHdr *>(aligned - sizeof(AlignedHdr));
  hdr->magic = ALIGNED_MAGIC;
  hdr->base = reinterpret_cast<void *>(raw);

  return reinterpret_cast<void *>(aligned);
}

// abort / pure virtual / atexit
[[noreturn]] void abort() {
  for (;;)
    __asm__ __volatile__("hlt");
}
[[noreturn]] void __cxa_pure_virtual() { abort(); }
void *__dso_handle = (void *)&__dso_handle;
int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }

// libc++ verbose abort needs minimal stdio
struct __FILE {};
FILE *stderr = (FILE *)0;
int vfprintf(FILE *, const char *, va_list) { return 0; }
int fputc(int, FILE *) { return 0; }
size_t fwrite(const void *, size_t, size_t, FILE *) { return 0; }

} // extern "C"

// -------- Basic operator new/delete (noexcept, no-throw) ----------
void *operator new(size_t n) { return kmalloc(n); }
void *operator new[](size_t n) { return kmalloc(n); }
void operator delete(void *p) noexcept {
  if (p)
    kfree(p);
}
void operator delete[](void *p) noexcept {
  if (p)
    kfree(p);
}

// Sized delete (C++14/17); на 32-битах clang мажет как `unsigned int`
void operator delete(void *p, size_t) noexcept {
  if (p)
    kfree(p);
}
void operator delete[](void *p, size_t) noexcept {
  if (p)
    kfree(p);
}
#if defined(__SIZEOF_SIZE_T__) && defined(__SIZEOF_INT__) &&                   \
    (__SIZEOF_SIZE_T__ != __SIZEOF_INT__)
void operator delete(void *p, unsigned int) noexcept {
  if (p)
    kfree(p);
}
void operator delete[](void *p, unsigned int) noexcept {
  if (p)
    kfree(p);
}
#endif

// Aligned new/delete (C++17); добавьте, если компилятор их требует
// #if defined(__cpp_aligned_new) && __cpp_aligned_new >= 201606
// void *operator new(size_t n, std::align_val_t) noexcept { return kmalloc(n);
// } void *operator new[](size_t n, std::align_val_t) noexcept { return
// kmalloc(n); } void operator delete(void *p, std::align_val_t) noexcept {
//   if (p)
//     kfree(p);
// }
// void operator delete[](void *p, std::align_val_t) noexcept {
//   if (p)
//     kfree(p);
// }
// void operator delete(void *p, size_t, std::align_val_t) noexcept {
//   if (p)
//     kfree(p);
// }
// void operator delete[](void *p, size_t, std::align_val_t) noexcept {
//   if (p)
//     kfree(p);
// }
// #endif
