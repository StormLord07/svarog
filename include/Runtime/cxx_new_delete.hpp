// runtime/cxx_new_delete.cpp
#include "kernel/memory/physical/MemoryManager.hpp"
#include <cstddef>
#include <new>

// -----------------------------------------------------------------------------
// Внешний С-API, на которые опираются операторы new/delete.
// kalloc/kfree делегируют в ваш менеджер памяти.
// -----------------------------------------------------------------------------
extern "C" void *kalloc(std::size_t n) {
  // Замените имена вызовов ниже на ваши реальные:
  //   alloc(n) / free(p) / allocate(n) / deallocate(p) — как у вас в Manager.
  return kernel_name::memory::physical::Manager::instance().alloc(n);
}

extern "C" void kfree(void *p) {
  if (!p)
    return;
  kernel_name::memory::physical::Manager::instance().free(p);
}

// -----------------------------------------------------------------------------
// Глобальные операторы new/delete.
// При нехватке памяти обычный new бросает std::bad_alloc.
// Если в ядре исключения отключены — замените throw на __builtin_trap().
// -----------------------------------------------------------------------------
void *operator new(std::size_t n) {
  if (void *p = kalloc(n))
    return p;
  throw std::bad_alloc();
}

void operator delete(void *p) noexcept {
  if (p)
    kfree(p);
}

void *operator new[](std::size_t n) { return ::operator new(n); }

void operator delete[](void *p) noexcept { ::operator delete(p); }

void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }

// -----------------------------------------------------------------------------
// Выравненные перегрузки (C++17 aligned new/delete).
// Если менеджер сам обеспечивает достаточное выравнивание, можно просто
// проксировать к обычным new/delete. Иначе — сделайте kalloc_aligned.
// -----------------------------------------------------------------------------
void *operator new(std::size_t n, std::align_val_t) {
  return ::operator new(n);
}

void operator delete(void *p, std::align_val_t) noexcept {
  ::operator delete(p);
}

void *operator new[](std::size_t n, std::align_val_t a) {
  return ::operator new(n, a);
}

void operator delete[](void *p, std::align_val_t a) noexcept {
  ::operator delete(p, a);
}

// -----------------------------------------------------------------------------
// nothrow-варианты: некоторые реализации стандартной библиотеки ожидают их.
// -----------------------------------------------------------------------------
void *operator new(std::size_t n, const std::nothrow_t &) noexcept {
  return kalloc(n);
}

void *operator new[](std::size_t n, const std::nothrow_t &) noexcept {
  return kalloc(n);
}

void operator delete(void *p, const std::nothrow_t &) noexcept { kfree(p); }

void operator delete[](void *p, const std::nothrow_t &) noexcept { kfree(p); }

void *operator new(std::size_t n, std::align_val_t,
                   const std::nothrow_t &) noexcept {
  return kalloc(n);
}

void *operator new[](std::size_t n, std::align_val_t,
                     const std::nothrow_t &) noexcept {
  return kalloc(n);
}

void operator delete(void *p, std::align_val_t,
                     const std::nothrow_t &) noexcept {
  kfree(p);
}

void operator delete[](void *p, std::align_val_t,
                       const std::nothrow_t &) noexcept {
  kfree(p);
}
