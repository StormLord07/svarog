#include <runtime/mem.h>

extern "C" {

void *memset(void *dst, int value, size_t n) noexcept {
  auto *p = static_cast<std::uint8_t *>(dst);
  while (n--) {
    *p++ = static_cast<std::uint8_t>(value);
  }
  return dst;
}

void *memcpy(void *dst, const void *src, size_t n) noexcept {
  auto *D = static_cast<std::uint8_t *>(dst);
  const auto *S = static_cast<const std::uint8_t *>(src);
  while (n--) {
    *D++ = *S++;
  }
  return dst;
}

void *memmove(void *dst, const void *src, size_t n) noexcept {
  auto *D = static_cast<std::uint8_t *>(dst);
  const auto *S = static_cast<const std::uint8_t *>(src);

  std::uintptr_t d = reinterpret_cast<std::uintptr_t>(D);
  std::uintptr_t s = reinterpret_cast<std::uintptr_t>(S);

  if (d < s) {
    while (n--) {
      *D++ = *S++;
    }
  } else {
    D += n;
    S += n;
    while (n--)
      *--D = *--S;
  }
  return dst;
}

int memcmp(const void *a, const void *b, size_t n) noexcept {
  const auto *A = static_cast<const std::uint8_t *>(a);
  const auto *B = static_cast<const std::uint8_t *>(b);
  for (; n--; ++A, ++B) {
    if (*A != *B) {
      return int(*A) - int(*B);
    }
  }
  return 0;
}

} // extern "C"
