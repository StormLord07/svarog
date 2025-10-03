#pragma once
#include <stdint.h>

namespace kernel_name::memory::physical::multiboot2 {
struct Info;
}

namespace vga {
void init(uint8_t attr = 0x07,
          const kernel_name::memory::physical::multiboot2::Info *mb2 = nullptr);
void clear();
void put(char c);
void write(const char *s);
void move_cursor(uint16_t x, uint16_t y);
void set_attr(uint8_t attr);
void set_xy(uint16_t x, uint16_t y);
uint16_t x();
uint16_t y();

// динамические размеры "текстовой" сетки
uint16_t width();
uint16_t height();

// helpers
void scroll_if_needed();
void put_at(uint16_t x, uint16_t y, char c);
} // namespace vga
