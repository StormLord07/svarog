#pragma once
#include <cstddef>
#include <cstdint>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// Your kernel allocator (provided elsewhere)
void *kmalloc(size_t n);
void kfree(void *p);

// ---- C runtime shims that libc++ / libc++abi expect ----

// Minimal memory/stdio pieces used by libc++(abi)
// (We only declare them here; definitions are in crt_shim.cpp)
void *malloc(size_t n);
void free(void *p);
void *aligned_alloc(size_t align, size_t size);

[[noreturn]] void abort(void);

// Pure-virtual call handler and atexit (trivial)
[[noreturn]] void __cxa_pure_virtual(void);
extern void *__dso_handle;
int __cxa_atexit(void (*f)(void *), void *arg, void *dso);

// Very small stdio stubs needed by libc++ verbose abort path
struct __FILE;
typedef struct __FILE FILE;
extern FILE *stderr;
int vfprintf(FILE *stream, const char *fmt, __builtin_va_list ap);
int fputc(int ch, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

// -------- Basic operator new/delete (noexcept, no-throw) ----------
void *operator new(size_t n);
void *operator new[](size_t n);
void operator delete(void *p) noexcept;
void operator delete[](void *p) noexcept;

void operator delete(void *p, size_t) noexcept;
void operator delete[](void *p, size_t) noexcept;
// Sized delete (C++14/17); на 32-битах clang мажет как `unsigned int`
#if defined(__SIZEOF_SIZE_T__) && defined(__SIZEOF_INT__) &&                   \
    (__SIZEOF_SIZE_T__ != __SIZEOF_INT__)
void operator delete(void *p, unsigned int) noexcept;
void operator delete[](void *p, unsigned int) noexcept;
#endif

// Nothrow new/delete (иногда используются контейнерами)
// #include <new>

// // Aligned new/delete (C++17); добавьте, если компилятор их требует
// #if defined(__cpp_aligned_new) && __cpp_aligned_new >= 201606
// void *operator new(size_t n, std::align_val_t);
// void *operator new[](size_t n, std::align_val_t);
// void operator delete(void *p, std::align_val_t) noexcept;
// void operator delete[](void *p, std::align_val_t) noexcept;
// void operator delete(void *p, size_t, std::align_val_t) noexcept;
// void operator delete[](void *p, size_t, std::align_val_t) noexcept;
// #endif

// -------- libc++ abort hook --------
// libc++ при отсутствии исключений дергает эту функцию.
namespace std {
inline namespace __1 {
__attribute__((noreturn)) void __libcpp_verbose_abort(const char * /*fmt*/,
                                                      ...) {
  // Здесь можно вывести что-то в вашу консоль/serial, если уже есть.
  for (;;) {
    __asm__ __volatile__("cli; hlt");
  }
}
} // namespace __1
} // namespace std
#endif
