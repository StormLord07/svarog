#pragma once
#include <stddef.h>
#include <stdint.h>


namespace kb {
bool poll_char(
    char &out); // returns true if an ASCII char is available (simple US layout)
bool poll_key(uint8_t &sc); // raw scancode if you want to extend
} // namespace kb
