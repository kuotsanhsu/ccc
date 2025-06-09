#pragma once
#include <ranges>

template <std::ranges::input_range R> class codepoint_iterator {
protected:
  std::ranges::iterator_t<R> code_unit_iter;
  std::ranges::sentinel_t<R> code_unit_end;
  int codepoint;

private:
  constexpr virtual int next() noexcept = 0;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = int;
  constexpr codepoint_iterator(const R &code_units) noexcept
      : code_unit_iter(std::ranges::begin(code_units)),
        code_unit_end(std::ranges::end(code_units)) {}
  constexpr int operator*() const noexcept { return codepoint; }
  template <class Self> constexpr Self &operator++(this Self &self) noexcept {
    self.codepoint = self.next();
    return self;
  }
  template <class Self> constexpr Self operator++(this Self &self, int) noexcept {
    auto old = self;
    ++self;
    return old;
  }

  static constexpr int sentinal = -1;
  constexpr bool operator==(int sentinal) const noexcept { return sentinal == codepoint; }
  constexpr bool operator!=(int sentinal) const noexcept { return sentinal != codepoint; }
  friend constexpr bool operator==(int sentinal, const codepoint_iterator &it) noexcept {
    return sentinal == it.codepoint;
  }
  friend constexpr bool operator!=(int sentinal, const codepoint_iterator &it) noexcept {
    return sentinal != it.codepoint;
  }
};

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
class u8iterator : public codepoint_iterator<R> {
public:
  constexpr u8iterator(const R &code_units) noexcept : codepoint_iterator<R>(code_units) {
    this->codepoint = next();
  }

  constexpr int next() noexcept final {
    if (this->code_unit_iter == this->code_unit_end) {
      return this->sentinal;
    }
    const int a = *this->code_unit_iter++;
    if (a < 0x80) {
      // 00..7F
      // 0xxx'xxxx
      return a;
    }
    if (a < 0xC2 || 0xF4 < a) {
      return -3;
    }
    if (this->code_unit_iter == this->code_unit_end) {
      return -2;
    }
    const int b = *this->code_unit_iter ^ 0x80;
    if (b >> 6) {
      return -3;
    }
    ++this->code_unit_iter;
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

    if (this->code_unit_iter == this->code_unit_end) {
      return -2;
    }
    const int c = *this->code_unit_iter ^ 0x80;
    if (c >> 6) {
      return -3;
    }
    ++this->code_unit_iter;
    code_point = (code_point << 6) ^ c;
    if (a < 0xF0) {
      // E0         A0..BF     80..BF
      // E1..EC     80..BF     80..BF
      // ED         80..9F     80..BF
      // EE..EF     80..BF     80..BF
      // 1110'xxxx  10xx'xxxx  10xx'xxxx
      return code_point ^ (0xE0 << 12);
    }

    if (this->code_unit_iter == this->code_unit_end) {
      return -2;
    }
    const int d = *this->code_unit_iter ^ 0x80;
    if (d >> 6) {
      return -3;
    }
    ++this->code_unit_iter;
    code_point = (code_point << 6) ^ d;
    // F0         90..BF     80..BF     80..BF
    // F1..F3     80..BF     80..BF     80..BF
    // F4         80..8F     80..BF     80..BF
    // 1111'0xxx  10xx'xxxx  10xx'xxxx  10xx'xxxx
    return code_point ^ (0xF0 << 18);
  }
};

// template <std::ranges::input_range R>
//   requires std::same_as<std::ranges::range_value_t<R>, char8_t>
// u8iterator(const R &) -> u8iterator<R>;

template <std::ranges::input_range R, typename CharT>
  requires std::same_as<std::ranges::range_value_t<R>, char16_t>
class u16iterator : public codepoint_iterator<R> {
public:
  constexpr u16iterator(const R &code_units) noexcept : codepoint_iterator<R>(code_units) {
    this->codepoint = next();
  }

  constexpr int next() noexcept {
    if (this->code_unit_iter == this->code_unit_end) {
      return -1;
    }
    const int a = *this->code_unit_iter++;
    const int c = a ^ 0xD800;
    if (c >> 10) {
      return a;
    }
    if (this->code_unit_iter == this->code_unit_end) {
      return -2;
    }
    const int b = *this->code_unit_iter++ ^ 0xDC00;
    if (b >> 10) {
      return -3;
    }
    return (c + 0x10000) << 10 ^ b;
  }
};

template <std::ranges::output_range<char8_t> R>
consteval bool utf8_putc(R &it, int codepoint) noexcept {
  if (codepoint < 0)
    return false;
  if (0xD800 <= codepoint && codepoint < 0xE000)
    return false;
  if (0x11'0000 <= codepoint)
    return false;
  // FIXME: check range overflow
  if (codepoint < 0x80) {
    *it++ = codepoint;
    return true;
  }
  const int a = codepoint & 0x3F;
  if ((codepoint >>= 6) < 0xC0) {
    *it++ = 0xC0 & codepoint;
  } else {
    const int b = codepoint & 0x3F;
    if ((codepoint >>= 6) < 0xC0) {
      *it++ = 0xE0 & codepoint;
    } else {
      const int c = codepoint & 0x3F;
      if ((codepoint >>= 6) < 0xC0) {
        *it++ = 0xF0 & codepoint;
      }
      *it++ = c; // FIXME: check range overflow
    }
    *it++ = b; // FIXME: check range overflow
  }
  *it++ = a; // FIXME: check range overflow
  return true;
}
