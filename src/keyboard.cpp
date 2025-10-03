#include "keyboard.hpp"

// Ports
static inline uint8_t inb(uint16_t port) {
  uint8_t val;
  asm volatile("inb %1,%0" : "=a"(val) : "Nd"(port));
  return val;
}
static inline void io_wait() { asm volatile("outb %%al,$0x80" ::"a"(0)); }

namespace {
bool shift = false;
bool e0 = false;

const char map_unshift[128] = {
    /*00*/ 0,    27,   '1', '2', '3',  '4',  '5',  '6',
    '7',         '8',  '9', '0', '-',  '=',  '\b',
    /*10*/ '\t', 'q',  'w', 'e', 'r',  't',  'y',  'u',
    'i',         'o',  'p', '[', ']',  '\n', 0,    'a',
    /*20*/ 's',  'd',  'f', 'g', 'h',  'j',  'k',  'l',
    ';',         '\'', '`', 0,   '\\', 'z',  'x',  'c',
    /*30*/ 'v',  'b',  'n', 'm', ',',  '.',  '/',  0,
    0,           0,    ' ', 0,   0,    0,    0,    0,
    /*40*/ 0};

const char map_shift[128] = {
    /*00*/ 0,    27,  '!', '@', '#', '$',  '%',  '^',
    '&',         '*', '(', ')', '_', '+',  '\b',
    /*10*/ '\t', 'Q', 'W', 'E', 'R', 'T',  'Y',  'U',
    'I',         'O', 'P', '{', '}', '\n', 0,    'A',
    /*20*/ 'S',  'D', 'F', 'G', 'H', 'J',  'K',  'L',
    ':',         '"', '~', 0,   '|', 'Z',  'X',  'C',
    /*30*/ 'V',  'B', 'N', 'M', '<', '>',  '?',  0,
    0,           0,   ' ', 0,   0,   0,    0,    0,
    /*40*/ 0};

bool read_scancode(uint8_t &sc) {
  // mask IRQ1 in PIC not needed for polling, just check output buffer
  if ((inb(0x64) & 1) == 0)
    return false;
  sc = inb(0x60);
  return true;
}
} // namespace

namespace kb {

bool poll_key(uint8_t &sc) { return read_scancode(sc); }

bool poll_char(char &out) {
  uint8_t sc;
  if (!read_scancode(sc))
    return false;

  if (sc == 0xE0) {
    e0 = true;
    return false;
  } // extended prefix
  bool release = sc & 0x80;
  uint8_t code = sc & 0x7F;

  // shift handling
  if (code == 0x2A || code == 0x36) { // LSHIFT/ RSHIFT
    shift = !release;
    e0 = false;
    return false;
  }

  if (release) {
    e0 = false;
    return false;
  } // ignore key-up for chars

  // minimal arrows support ignored here; extend using e0+codes if needed
  char c = shift ? map_shift[code] : map_unshift[code];
  if (c) {
    out = c;
    e0 = false;
    return true;
  }

  e0 = false;
  return false;
}

} // namespace kb
