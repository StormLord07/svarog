#pragma once
#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void *memset(void *dst, int value, size_t n) noexcept;
void *memcpy(void *dst, const void *src, size_t n) noexcept;
void *memmove(void *dst, const void *src, size_t n) noexcept;
int memcmp(const void *a, const void *b, size_t n) noexcept;

#ifdef __cplusplus
}
#endif