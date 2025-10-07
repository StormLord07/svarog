#include "vga.hpp"
#include "boot/loaders/Multiboot2.hpp"
#include <stddef.h>
#include <stdint.h>

// ---- внешний шрифт 8x16 (моноширинный, 256 символов) ----
// Объяви где-нибудь: `const uint8_t font8x16[256][16] = {...};`
static const uint8_t font8x16[256][16] = {
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x00,
    {
        0x00,
        0x00,
        0x7E,
        0x81,
        0xA5,
        0x81,
        0x81,
        0xBD,
        0x99,
        0x81,
        0x81,
        0x7E,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x01,
    {
        0x00,
        0x00,
        0x7E,
        0xFF,
        0xDB,
        0xFF,
        0xFF,
        0xC3,
        0xE7,
        0xFF,
        0xFF,
        0x7E,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x02,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x6C,
        0xFE,
        0xFE,
        0xFE,
        0xFE,
        0x7C,
        0x38,
        0x10,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x03,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x10,
        0x38,
        0x7C,
        0xFE,
        0x7C,
        0x38,
        0x10,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x04,
    {
        0x00,
        0x00,
        0x00,
        0x18,
        0x3C,
        0x3C,
        0xE7,
        0xE7,
        0xE7,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x05,
    {
        0x00,
        0x00,
        0x00,
        0x18,
        0x3C,
        0x7E,
        0xFF,
        0xFF,
        0x7E,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x06,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x3C,
        0x3C,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x07,
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xE7,
        0xC3,
        0xC3,
        0xE7,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
    }, // 0x08,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x3C,
        0x66,
        0x42,
        0x42,
        0x66,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x09,
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xC3,
        0x99,
        0xBD,
        0xBD,
        0x99,
        0xC3,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
    }, // 0x0A,
    {
        0x00,
        0x00,
        0x1E,
        0x0E,
        0x1A,
        0x32,
        0x78,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x78,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x0B,
    {
        0x00,
        0x00,
        0x3C,
        0x66,
        0x66,
        0x66,
        0x66,
        0x3C,
        0x18,
        0x7E,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x0C,
    {
        0x00,
        0x00,
        0x3F,
        0x33,
        0x3F,
        0x30,
        0x30,
        0x30,
        0x30,
        0x70,
        0xF0,
        0xE0,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x0D,
    {
        0x00,
        0x00,
        0x7F,
        0x63,
        0x7F,
        0x63,
        0x63,
        0x63,
        0x63,
        0x67,
        0xE7,
        0xE6,
        0xC0,
        0x00,
        0x00,
        0x00,
    }, // 0x0E,
    {
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0xDB,
        0x3C,
        0xE7,
        0x3C,
        0xDB,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x0F,
    {
        0x00,
        0x80,
        0xC0,
        0xE0,
        0xF0,
        0xF8,
        0xFE,
        0xF8,
        0xF0,
        0xE0,
        0xC0,
        0x80,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x10,
    {
        0x00,
        0x02,
        0x06,
        0x0E,
        0x1E,
        0x3E,
        0xFE,
        0x3E,
        0x1E,
        0x0E,
        0x06,
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x11,
    {
        0x00,
        0x00,
        0x18,
        0x3C,
        0x7E,
        0x18,
        0x18,
        0x18,
        0x7E,
        0x3C,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x12,
    {
        0x00,
        0x00,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x00,
        0x66,
        0x66,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x13,
    {
        0x00,
        0x00,
        0x7F,
        0xDB,
        0xDB,
        0xDB,
        0x7B,
        0x1B,
        0x1B,
        0x1B,
        0x1B,
        0x1B,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x14,
    {
        0x00,
        0x7C,
        0xC6,
        0x60,
        0x38,
        0x6C,
        0xC6,
        0xC6,
        0x6C,
        0x38,
        0x0C,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
    }, // 0x15,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xFE,
        0xFE,
        0xFE,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x16,
    {
        0x00,
        0x00,
        0x18,
        0x3C,
        0x7E,
        0x18,
        0x18,
        0x18,
        0x7E,
        0x3C,
        0x18,
        0x7E,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x17,
    {
        0x00,
        0x00,
        0x18,
        0x3C,
        0x7E,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x18,
    {
        0x00,
        0x00,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x7E,
        0x3C,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x19,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x0C,
        0xFE,
        0x0C,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x1A,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x30,
        0x60,
        0xFE,
        0x60,
        0x30,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x1B, esc
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xC0,
        0xC0,
        0xC0,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x1C,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x28,
        0x6C,
        0xFE,
        0x6C,
        0x28,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x1D,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x10,
        0x38,
        0x38,
        0x7C,
        0x7C,
        0xFE,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x1E,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0xFE,
        0xFE,
        0x7C,
        0x7C,
        0x38,
        0x38,
        0x10,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x1F,
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x20, ' '
    {
        0x00,
        0x00,
        0x18,
        0x3C,
        0x3C,
        0x3C,
        0x18,
        0x18,
        0x18,
        0x00,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x21, '!'
    {
        0x00,
        0x66,
        0x66,
        0x66,
        0x24,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x22, '"'
    {
        0x00,
        0x00,
        0x00,
        0x6C,
        0x6C,
        0xFE,
        0x6C,
        0x6C,
        0x6C,
        0xFE,
        0x6C,
        0x6C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x23, '#'
    {
        0x18,
        0x18,
        0x7C,
        0xC6,
        0xC2,
        0xC0,
        0x7C,
        0x06,
        0x06,
        0x86,
        0xC6,
        0x7C,
        0x18,
        0x18,
        0x00,
        0x00,
    }, // 0x24, '$'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0xC2,
        0xC6,
        0x0C,
        0x18,
        0x30,
        0x60,
        0xC6,
        0x86,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x25, '%'
    {
        0x00,
        0x00,
        0x38,
        0x6C,
        0x6C,
        0x38,
        0x76,
        0xDC,
        0xCC,
        0xCC,
        0xCC,
        0x76,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x26, '&'
    {
        0x00,
        0x30,
        0x30,
        0x30,
        0x60,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x27, '''
    {
        0x00,
        0x00,
        0x0C,
        0x18,
        0x30,
        0x30,
        0x30,
        0x30,
        0x30,
        0x30,
        0x18,
        0x0C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x28, '('
    {
        0x00,
        0x00,
        0x30,
        0x18,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x18,
        0x30,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x29, ')'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x66,
        0x3C,
        0xFF,
        0x3C,
        0x66,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x2A, '*'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x7E,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x2B, '+'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x18,
        0x30,
        0x00,
        0x00,
        0x00,
    }, // 0x2C, '
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x2D, '-'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x2E, '.'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x02,
        0x06,
        0x0C,
        0x18,
        0x30,
        0x60,
        0xC0,
        0x80,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x2F, '/'
    {
        0x00,
        0x00,
        0x38,
        0x6C,
        0xC6,
        0xC6,
        0xD6,
        0xD6,
        0xC6,
        0xC6,
        0x6C,
        0x38,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x30, '0'
    {
        0x00,
        0x00,
        0x18,
        0x38,
        0x78,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x7E,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x31, '1'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0x06,
        0x0C,
        0x18,
        0x30,
        0x60,
        0xC0,
        0xC6,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x32, '2'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0x06,
        0x06,
        0x3C,
        0x06,
        0x06,
        0x06,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x33, '3'
    {
        0x00,
        0x00,
        0x0C,
        0x1C,
        0x3C,
        0x6C,
        0xCC,
        0xFE,
        0x0C,
        0x0C,
        0x0C,
        0x1E,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x34, '4'
    {
        0x00,
        0x00,
        0xFE,
        0xC0,
        0xC0,
        0xC0,
        0xFC,
        0x06,
        0x06,
        0x06,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x35, '5'
    {
        0x00,
        0x00,
        0x38,
        0x60,
        0xC0,
        0xC0,
        0xFC,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x36, '6'
    {
        0x00,
        0x00,
        0xFE,
        0xC6,
        0x06,
        0x06,
        0x0C,
        0x18,
        0x30,
        0x30,
        0x30,
        0x30,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x37, '7'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0xC6,
        0x7C,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x38, '8'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0xC6,
        0x7E,
        0x06,
        0x06,
        0x06,
        0x0C,
        0x78,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x39, '9'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x3A, ':'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x18,
        0x18,
        0x30,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x3B, ';'
    {
        0x00,
        0x00,
        0x00,
        0x06,
        0x0C,
        0x18,
        0x30,
        0x60,
        0x30,
        0x18,
        0x0C,
        0x06,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x3C, '<'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x7E,
        0x00,
        0x00,
        0x7E,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x3D, '='
    {
        0x00,
        0x00,
        0x00,
        0x60,
        0x30,
        0x18,
        0x0C,
        0x06,
        0x0C,
        0x18,
        0x30,
        0x60,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x3E, '>'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0x0C,
        0x18,
        0x18,
        0x18,
        0x00,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x3F, '?'
    {
        0x00,
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0xDE,
        0xDE,
        0xDE,
        0xDC,
        0xC0,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x40, '@'
    {
        0x00,
        0x00,
        0x10,
        0x38,
        0x6C,
        0xC6,
        0xC6,
        0xFE,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x41, 'A'
    {
        0x00,
        0x00,
        0xFC,
        0x66,
        0x66,
        0x66,
        0x7C,
        0x66,
        0x66,
        0x66,
        0x66,
        0xFC,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x42, 'B'
    {
        0x00,
        0x00,
        0x3C,
        0x66,
        0xC2,
        0xC0,
        0xC0,
        0xC0,
        0xC0,
        0xC2,
        0x66,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x43, 'C'
    {
        0x00,
        0x00,
        0xF8,
        0x6C,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x6C,
        0xF8,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x44, 'D'
    {
        0x00,
        0x00,
        0xFE,
        0x66,
        0x62,
        0x68,
        0x78,
        0x68,
        0x60,
        0x62,
        0x66,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x45, 'E'
    {
        0x00,
        0x00,
        0xFE,
        0x66,
        0x62,
        0x68,
        0x78,
        0x68,
        0x60,
        0x60,
        0x60,
        0xF0,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x46, 'F'
    {
        0x00,
        0x00,
        0x3C,
        0x66,
        0xC2,
        0xC0,
        0xC0,
        0xDE,
        0xC6,
        0xC6,
        0x66,
        0x3A,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x47, 'G'
    {
        0x00,
        0x00,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xFE,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x48, 'H'
    {
        0x00,
        0x00,
        0x3C,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x49, 'I'
    {
        0x00,
        0x00,
        0x1E,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0xCC,
        0xCC,
        0xCC,
        0x78,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x4A, 'J'
    {
        0x00,
        0x00,
        0xE6,
        0x66,
        0x66,
        0x6C,
        0x78,
        0x78,
        0x6C,
        0x66,
        0x66,
        0xE6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x4B, 'K'
    {
        0x00,
        0x00,
        0xF0,
        0x60,
        0x60,
        0x60,
        0x60,
        0x60,
        0x60,
        0x62,
        0x66,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x4C, 'L'
    {
        0x00,
        0x00,
        0xC6,
        0xEE,
        0xFE,
        0xFE,
        0xD6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x4D, 'M'
    {
        0x00,
        0x00,
        0xC6,
        0xE6,
        0xF6,
        0xFE,
        0xDE,
        0xCE,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x4E, 'N'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x4F, 'O'
    {
        0x00,
        0x00,
        0xFC,
        0x66,
        0x66,
        0x66,
        0x7C,
        0x60,
        0x60,
        0x60,
        0x60,
        0xF0,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x50, 'P'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xD6,
        0xDE,
        0x7C,
        0x0C,
        0x0E,
        0x00,
        0x00,
    }, // 0x51, 'Q'
    {
        0x00,
        0x00,
        0xFC,
        0x66,
        0x66,
        0x66,
        0x7C,
        0x6C,
        0x66,
        0x66,
        0x66,
        0xE6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x52, 'R'
    {
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0x60,
        0x38,
        0x0C,
        0x06,
        0xC6,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x53, 'S'
    {
        0x00,
        0x00,
        0x7E,
        0x7E,
        0x5A,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x54, 'T'
    {
        0x00,
        0x00,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x55, 'U'
    {
        0x00,
        0x00,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x6C,
        0x38,
        0x10,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x56, 'V'
    {
        0x00,
        0x00,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xD6,
        0xD6,
        0xD6,
        0xFE,
        0xEE,
        0x6C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x57, 'W'
    {
        0x00,
        0x00,
        0xC6,
        0xC6,
        0x6C,
        0x7C,
        0x38,
        0x38,
        0x7C,
        0x6C,
        0xC6,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x58, 'X'
    {
        0x00,
        0x00,
        0x66,
        0x66,
        0x66,
        0x66,
        0x3C,
        0x18,
        0x18,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x59, 'Y'
    {
        0x00,
        0x00,
        0xFE,
        0xC6,
        0x86,
        0x0C,
        0x18,
        0x30,
        0x60,
        0xC2,
        0xC6,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x5A, 'Z'
    {
        0x00,
        0x00,
        0x3C,
        0x30,
        0x30,
        0x30,
        0x30,
        0x30,
        0x30,
        0x30,
        0x30,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x5B, '['
    {
        0x00,
        0x00,
        0x00,
        0x80,
        0xC0,
        0xE0,
        0x70,
        0x38,
        0x1C,
        0x0E,
        0x06,
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x5C, '\'
    {
        0x00,
        0x00,
        0x3C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x5D, ']'
    {
        0x10,
        0x38,
        0x6C,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x5E, '^'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xFF,
        0x00,
        0x00,
    }, // 0x5F, '_'
    {
        0x30,
        0x30,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x60, '`'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x78,
        0x0C,
        0x7C,
        0xCC,
        0xCC,
        0xCC,
        0x76,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x61, 'a'
    {
        0x00,
        0x00,
        0xE0,
        0x60,
        0x60,
        0x78,
        0x6C,
        0x66,
        0x66,
        0x66,
        0x66,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x62, 'b'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC0,
        0xC0,
        0xC0,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x63, 'c'
    {
        0x00,
        0x00,
        0x1C,
        0x0C,
        0x0C,
        0x3C,
        0x6C,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x76,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x64, 'd'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xFE,
        0xC0,
        0xC0,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x65, 'e'
    {
        0x00,
        0x00,
        0x38,
        0x6C,
        0x64,
        0x60,
        0xF0,
        0x60,
        0x60,
        0x60,
        0x60,
        0xF0,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x66, 'f'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x76,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x7C,
        0x0C,
        0xCC,
        0x78,
        0x00,
    }, // 0x67, 'g'
    {
        0x00,
        0x00,
        0xE0,
        0x60,
        0x60,
        0x6C,
        0x76,
        0x66,
        0x66,
        0x66,
        0x66,
        0xE6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x68, 'h'
    {
        0x00,
        0x00,
        0x18,
        0x18,
        0x00,
        0x38,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x69, 'i'
    {
        0x00,
        0x00,
        0x06,
        0x06,
        0x00,
        0x0E,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x66,
        0x66,
        0x3C,
        0x00,
    }, // 0x6A, 'j'
    {
        0x00,
        0x00,
        0xE0,
        0x60,
        0x60,
        0x66,
        0x6C,
        0x78,
        0x78,
        0x6C,
        0x66,
        0xE6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x6B, 'k'
    {
        0x00,
        0x00,
        0x38,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x3C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x6C, 'l'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xEC,
        0xFE,
        0xD6,
        0xD6,
        0xD6,
        0xD6,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x6D, 'm'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xDC,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x6E, 'n'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x7C,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x6F, 'o'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xDC,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x7C,
        0x60,
        0x60,
        0xF0,
        0x00,
    }, // 0x70, 'p'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x76,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x7C,
        0x0C,
        0x0C,
        0x1E,
        0x00,
    }, // 0x71, 'q'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xDC,
        0x76,
        0x66,
        0x60,
        0x60,
        0x60,
        0xF0,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x72, 'r'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x7C,
        0xC6,
        0x60,
        0x38,
        0x0C,
        0xC6,
        0x7C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x73, 's'
    {
        0x00,
        0x00,
        0x10,
        0x30,
        0x30,
        0xFC,
        0x30,
        0x30,
        0x30,
        0x30,
        0x36,
        0x1C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x74, 't'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x76,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x75, 'u'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x3C,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x76, 'v'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xC6,
        0xC6,
        0xD6,
        0xD6,
        0xD6,
        0xFE,
        0x6C,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x77, 'w'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xC6,
        0x6C,
        0x38,
        0x38,
        0x38,
        0x6C,
        0xC6,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x78, 'x'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0xC6,
        0x7E,
        0x06,
        0x0C,
        0xF8,
        0x00,
    }, // 0x79, 'y'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xFE,
        0xCC,
        0x18,
        0x30,
        0x60,
        0xC6,
        0xFE,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x7A, 'z'
    {
        0x00,
        0x00,
        0x0E,
        0x18,
        0x18,
        0x18,
        0x70,
        0x18,
        0x18,
        0x18,
        0x18,
        0x0E,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x7B, '{'
    {
        0x00,
        0x00,
        0x18,
        0x18,
        0x18,
        0x18,
        0x00,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x7C, '|'
    {
        0x00,
        0x00,
        0x70,
        0x18,
        0x18,
        0x18,
        0x0E,
        0x18,
        0x18,
        0x18,
        0x18,
        0x70,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x7D, '}'
    {
        0x00,
        0x00,
        0x76,
        0xDC,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x7E, '~'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 0x7F, delete
};

// ------------------------ ВНУТРЕННЕЕ СОСТОЯНИЕ ------------------------
namespace {

// --- общий стейт ---
uint16_t curX = 0, curY = 0;
uint8_t g_attr = 0x07;

// --- текстовый VGA 0xB8000 ---
volatile uint16_t *VGA = (uint16_t *)0xB8000;
static inline uint16_t mkcell(char c) {
  return uint16_t(uint8_t(c)) | (uint16_t(g_attr) << 8);
}

// порт I/O курсора
static inline void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %0,%1" ::"a"(val), "Nd"(port));
}
inline void hw_cursor_update(uint16_t x, uint16_t y, uint16_t W) {
  uint16_t pos = y * W + x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// --- framebuffer режим ---
bool use_fb = false;
uint8_t *fb_base = nullptr;
uint32_t fb_pitch = 0, fb_W = 0, fb_H = 0;
uint8_t fb_bpp = 0;

// логические размеры "текстовой сетки" в fb
static constexpr uint32_t GLYPH_W = 8;
static constexpr uint32_t GLYPH_H = 16;
uint16_t grid_W = 80; // по умолчанию
uint16_t grid_H = 25;

// простейший бэкбуфер символов для fb, чтобы скроллить быстро
static constexpr uint16_t MAX_W = 256;
static constexpr uint16_t MAX_H = 128;
char screen_buf[MAX_H][MAX_W]; // ~32 KiB

// -------------------- МАСКИ ЦВЕТА (Multiboot2 RGB) --------------------
struct RGBMasks {
  uint8_t r_pos = 0, r_size = 0;
  uint8_t g_pos = 0, g_size = 0;
  uint8_t b_pos = 0, b_size = 0;
  uint8_t a_pos = 0, a_size = 0; // редко присутствует; оставим на всякий случай
};

RGBMasks fb_masks{};

inline RGBMasks parse_rgb_masks(
    const kernel_name::memory::physical::multiboot2::TagFramebuffer &fb) {
  // В Multiboot2 для FBType::RGB сразу после заголовка идут
  // пары (pos,size) для R,G,B (и иногда A). Порядок:
  // r_pos,r_size,g_pos,g_size,b_pos,b_size,(a_pos,a_size?)
  RGBMasks m{};
  const uint8_t *p = reinterpret_cast<const uint8_t *>(&fb) + sizeof(fb);
  const size_t payload = (fb.size > sizeof(fb)) ? (fb.size - sizeof(fb)) : 0;

  if (payload >= 6) {
    m.r_pos = p[0];
    m.r_size = p[1];
    m.g_pos = p[2];
    m.g_size = p[3];
    m.b_pos = p[4];
    m.b_size = p[5];
  }
  if (payload >= 8) {
    m.a_pos = p[6];
    m.a_size = p[7];
  }
  return m;
}

// Упаковать ARGB8888 в нативное представление по маскам.
inline uint32_t pack_color(uint32_t argb, const RGBMasks &m) {
  auto scale = [](uint8_t x, uint8_t bits) -> uint32_t {
    if (!bits)
      return 0;
    uint32_t maxv = (1u << bits) - 1u;
    // линейно масштабируем 0..255 -> 0..(2^bits-1) с округлением
    return (uint32_t(x) * maxv + 127u) / 255u;
  };
  uint8_t a = (argb >> 24) & 0xFF;
  uint8_t r = (argb >> 16) & 0xFF;
  uint8_t g = (argb >> 8) & 0xFF;
  uint8_t b = (argb >> 0) & 0xFF;

  uint32_t v = 0;
  v |= scale(r, m.r_size) << m.r_pos;
  v |= scale(g, m.g_size) << m.g_pos;
  v |= scale(b, m.b_size) << m.b_pos;
  if (m.a_size)
    v |= scale(a, m.a_size) << m.a_pos;
  return v;
}

inline void fb_putpixel(uint32_t x, uint32_t y, uint32_t argb) {
  if (x >= fb_W || y >= fb_H)
    return;

  uint8_t *p = fb_base + y * fb_pitch + x * (fb_bpp / 8);
  uint32_t v = pack_color(argb, fb_masks);

  switch (fb_bpp) {
  case 16: { // например RGB565
    *reinterpret_cast<uint16_t *>(p) = static_cast<uint16_t>(v);
  } break;
  case 24: { // 3 байта, little-endian младшие 24 бита
    p[0] = uint8_t((v >> 0) & 0xFF);
    p[1] = uint8_t((v >> 8) & 0xFF);
    p[2] = uint8_t((v >> 16) & 0xFF);
  } break;
  case 32: {
    *reinterpret_cast<uint32_t *>(p) = v;
  } break;
  default: /* неподдерживаемый bpp — ничего не делаем */
    break;
  }
}

inline void fb_blit_glyph(uint16_t gx, uint16_t gy, char ch, uint32_t fg,
                          uint32_t bg) {
  const uint32_t x0 = gx * GLYPH_W;
  const uint32_t y0 = gy * GLYPH_H;
  const uint8_t *rows = font8x16[(uint8_t)ch];
  for (uint32_t r = 0; r < GLYPH_H; ++r) {
    uint8_t row = rows[r];
    for (uint32_t c = 0; c < GLYPH_W; ++c) {
      bool bit = (row & (0x80 >> c));
      fb_putpixel(x0 + c, y0 + r, bit ? fg : bg);
    }
  }
}

// Очистка фреймбуфера любым цветом
inline void fb_full_clear(uint32_t argb) {
  // заполним построчно с учётом masks/bpp
  // Чтобы не упаковывать цвет на каждый пиксель, упакуем 1 раз и разложим.
  uint32_t packed = pack_color(argb, fb_masks);
  for (uint32_t y = 0; y < fb_H; ++y) {
    uint8_t *line = fb_base + y * fb_pitch;
    switch (fb_bpp) {
    case 16: {
      uint16_t v16 = static_cast<uint16_t>(packed);
      for (uint32_t x = 0; x < fb_W; ++x)
        reinterpret_cast<uint16_t *>(line)[x] = v16;
    } break;
    case 24: {
      uint8_t b0 = uint8_t((packed >> 0) & 0xFF);
      uint8_t b1 = uint8_t((packed >> 8) & 0xFF);
      uint8_t b2 = uint8_t((packed >> 16) & 0xFF);
      for (uint32_t x = 0; x < fb_W; ++x) {
        line[x * 3 + 0] = b0;
        line[x * 3 + 1] = b1;
        line[x * 3 + 2] = b2;
      }
    } break;
    case 32: {
      for (uint32_t x = 0; x < fb_W; ++x)
        reinterpret_cast<uint32_t *>(line)[x] = packed;
    } break;
    default:
      break;
    }
  }
}

// преобразование атрибута VGA (0x07 и т.п.) в цвета ARGB (простая палитра)
inline uint32_t idx_to_argb(uint8_t i4) {
  static const uint32_t pal[16] = {
      0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA, 0xFFAA0000, 0xFFAA00AA,
      0xFFAA5500, 0xFFAAAAAA, 0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
      0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF};
  return pal[i4 & 0x0F];
}

inline void fb_draw_char_at(uint16_t x, uint16_t y, char ch) {
  uint8_t fg_i = g_attr & 0x0F;
  uint8_t bg_i = (g_attr >> 4) & 0x0F;
  fb_blit_glyph(x, y, ch, idx_to_argb(fg_i), idx_to_argb(bg_i));
}

} // namespace

// ------------------------------ API ------------------------------
namespace vga {

uint16_t width() { return grid_W; }
uint16_t height() { return grid_H; }

void init(uint8_t attr,
          const kernel_name::memory::physical::multiboot2::Info *mb2) {
  g_attr = attr;
  use_fb = false;

  if (mb2) {
    for (const kernel_name::memory::physical::multiboot2::Tag &tag : *mb2) {
      if (tag.type ==
          kernel_name::memory::physical::multiboot2::TYPE::MB2_TAG_END)
        break;

      if (tag.type == kernel_name::memory::physical::multiboot2::TYPE::
                          MB2_TAG_FRAMEBUFFER) {
        const auto &fb = *reinterpret_cast<
            const kernel_name::memory::physical::multiboot2::TagFramebuffer *>(
            &tag);
        if (fb.fbtype ==
            kernel_name::memory::physical::multiboot2::FBType::EGAText) {
          // Текстовый режим из тега MB2: адрес, кол-во колонок/строк
          VGA = (uint16_t *)(uintptr_t)fb.addr;
          grid_W = (uint16_t)fb.width;  // колонки
          grid_H = (uint16_t)fb.height; // строки
          use_fb = false;               // принудительно текст
          break;
        }
        if (fb.fbtype ==
                kernel_name::memory::physical::multiboot2::FBType::RGB &&
            (fb.bpp == 16 || fb.bpp == 24 || fb.bpp == 32)) {
          fb_base = (uint8_t *)(uintptr_t)fb.addr;
          fb_pitch = fb.pitch;
          fb_W = fb.width;
          fb_H = fb.height;
          fb_bpp = fb.bpp;
          fb_masks = parse_rgb_masks(fb);
          use_fb = true;

          grid_W = (uint16_t)(fb_W / GLYPH_W);
          grid_H = (uint16_t)(fb_H / GLYPH_H);
        }
        break;
      }
    }
  }

  clear();
}

void clear() {
  curX = curY = 0;
  if (use_fb) {
    fb_full_clear(0xFF000000); // чёрный
    // очистим буфер символов
    for (uint16_t y = 0; y < MAX_H; ++y)
      for (uint16_t x = 0; x < MAX_W; ++x)
        screen_buf[y][x] = ' ';
  } else {
    const uint16_t W = width(), H = height();
    for (size_t i = 0; i < (size_t)W * H; ++i)
      VGA[i] = mkcell(' ');
    hw_cursor_update(curX, curY, W);
  }
}

void set_attr(uint8_t a) { g_attr = a; }

void set_xy(uint16_t x, uint16_t y) {
  uint16_t W = width(), H = height();
  curX = (x < W) ? x : (W ? W - 1 : 0);
  curY = (y < H) ? y : (H ? H - 1 : 0);
  if (!use_fb)
    hw_cursor_update(curX, curY, /*W*/ width());
}

uint16_t x() { return curX; }
uint16_t y() { return curY; }

void scroll_if_needed() {
  uint16_t W = width(), H = height();
  if (curY < H)
    return;

  if (use_fb) {
    for (uint16_t r = 1; r < H; ++r)
      for (uint16_t c = 0; c < W; ++c)
        screen_buf[r - 1][c] = screen_buf[r][c];
    for (uint16_t c = 0; c < W; ++c)
      screen_buf[H - 1][c] = ' ';

    for (uint16_t r = 0; r < H; ++r)
      for (uint16_t c = 0; c < W; ++c)
        fb_draw_char_at(c, r, screen_buf[r][c]);

    curY = H - 1;
  } else {
    const uint16_t Wt = width(), Ht = height();
    for (uint16_t r = 1; r < Ht; ++r)
      for (uint16_t c = 0; c < Wt; ++c)
        VGA[(r - 1) * Wt + c] = VGA[r * Wt + c];
    for (uint16_t c = 0; c < Wt; ++c)
      VGA[(Ht - 1) * Wt + c] = mkcell(' ');
    curY = Ht - 1;
  }
}

void put_at(uint16_t X, uint16_t Y, char ch) {
  uint16_t W = width(), H = height();
  if (X >= W || Y >= H)
    return;

  if (use_fb) {
    screen_buf[Y][X] = ch;
    fb_draw_char_at(X, Y, ch);
  } else {
    VGA[Y * width() + X] = mkcell(ch);
  }
}

void put(char c) {
  if (c == '\n') {
    curX = 0;
    ++curY;
    scroll_if_needed();
  } else if (c == '\r') {
    curX = 0;
  } else if (c == '\t') {
    uint16_t W = width();
    curX = (uint16_t)((curX + 4) & ~3);
    if (curX >= W) {
      curX = 0;
      ++curY;
      scroll_if_needed();
    }
  } else {
    put_at(curX, curY, c);
    if (++curX >= width()) {
      curX = 0;
      ++curY;
      scroll_if_needed();
    }
  }
  if (!use_fb)
    hw_cursor_update(curX, curY, /*W*/ width());
}

void write(const char *s) {
  while (*s)
    put(*s++);
}
void move_cursor(uint16_t X, uint16_t Y) { set_xy(X, Y); }

} // namespace vga