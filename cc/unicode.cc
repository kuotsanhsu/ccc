#include "unicode.hh"

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
constexpr int u8iterator<R>::next() noexcept {
  if (curr == end) {
    return -1;
  }
  const int a = *curr++;
  if (a < 0x80) {
    // 00..7F
    // 0xxx'xxxx
    return a;
  }
  if (a < 0xC2 || 0xF4 < a) {
    return -3;
  }
  if (curr == end) {
    return -2;
  }
  const int b = *curr ^ 0x80;
  if (b >> 6) {
    return -3;
  }
  ++curr;
  int code_point = a << 6 ^ b;
  if (a < 0xE0) {
    // C2..DF     80..BF
    // 110x'xxxx  10xx'xxxx
    return code_point ^ (0xC0 << 6);
  }

  switch (a) {
  case 0xE0: // E0  A0..BF  80..BF
    if (b < 0x20) {
      return -3;
    }
    break;
  case 0xED: // ED  80..9F  80..BF
    if (b >= 0x20) {
      return -3;
    }
    break;
  case 0xF0: // F0  90..BF  80..BF  80..BF
    if (b < 0x10) {
      return -3;
    }
    break;
  case 0xF4: // F4  80..8F  80..BF  80..BF
    if (b >= 0x10) {
      return -3;
    }
    break;
  }

  if (curr == end) {
    return -2;
  }
  const int c = *curr ^ 0x80;
  if (c >> 6) {
    return -3;
  }
  ++curr;
  code_point = (code_point << 6) ^ c;
  if (a < 0xF0) {
    // E0         A0..BF     80..BF
    // E1..EC     80..BF     80..BF
    // ED         80..9F     80..BF
    // EE..EF     80..BF     80..BF
    // 1110'xxxx  10xx'xxxx  10xx'xxxx
    return code_point ^ (0xE0 << 12);
  }

  if (curr == end) {
    return -2;
  }
  const int d = *curr ^ 0x80;
  if (d >> 6) {
    return -3;
  }
  ++curr;
  code_point = (code_point << 6) ^ d;
  // F0         90..BF     80..BF     80..BF
  // F1..F3     80..BF     80..BF     80..BF
  // F4         80..8F     80..BF     80..BF
  // 1111'0xxx  10xx'xxxx  10xx'xxxx  10xx'xxxx
  return code_point ^ (0xF0 << 18);
}
