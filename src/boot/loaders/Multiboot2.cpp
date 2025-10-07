#include "boot/loaders/Multiboot2.hpp"

namespace kernel_name::memory::physical::multiboot2 {

const Tag *Tag::next() const {
  auto p = reinterpret_cast<std::uintptr_t>(this) + size;
  p = (p + 7) & ~std::uintptr_t(7); // 8-byte align per spec
  return reinterpret_cast<const Tag *>(p);
}

} // namespace kernel_name::memory::physical::multiboot2