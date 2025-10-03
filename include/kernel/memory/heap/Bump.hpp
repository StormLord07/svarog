#pragma once
#include <cstddef>
#include <cstdint>

namespace kernel_name::memory::heap {

class Bump {
public:
  Bump() = default;
  Bump(void *start, std::size_t bytes) { init(start, bytes); }
  void init(void *start, std::size_t bytes);

  void *alloc(std::size_t n, std::size_t align = alignof(std::max_align_t));
  void free(void *) {} // no-op

private:
  std::uint8_t *cur_ = nullptr;
  std::uint8_t *end_ = nullptr;
};

} // namespace kernel_name::memory::heap
