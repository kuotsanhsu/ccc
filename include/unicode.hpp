#pragma once
#include "assert.hpp"
#include <ranges>

template <typename R, typename T>
concept code_unit_sequence =
    std::ranges::input_range<R> && std::same_as<std::ranges::range_value_t<R>, T>;

template <typename R>
concept codepoint_sequence = code_unit_sequence<R, int>;

template <typename R>
concept utf8_code_unit_sequence = code_unit_sequence<R, char8_t>;

template <typename R>
concept utf16_code_unit_sequence = code_unit_sequence<R, char16_t>;

template <typename R>
concept utf32_code_unit_sequence = code_unit_sequence<R, char32_t>;

template <utf8_code_unit_sequence R>
class codepoint_view : public std::ranges::view_interface<codepoint_view<R>> {
  R code_units;

public:
  class iterator;
  constexpr codepoint_view(R code_units) noexcept : code_units(std::move(code_units)) {}
  constexpr iterator begin() const {
    return {std::ranges::begin(code_units), std::ranges::end(code_units)};
  }
  [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
    return std::default_sentinel;
  }
};

template <utf8_code_unit_sequence R> class codepoint_view<R>::iterator {
  std::ranges::iterator_t<R> code_unit_iter;
  std::ranges::sentinel_t<R> code_unit_end;
  int codepoint;
  static constexpr int eof = -1;
  /** Could throw in the case of reading from stdin. */
  constexpr int next();

public:
  using difference_type = std::ptrdiff_t;
  using value_type = int;
  constexpr iterator(std::ranges::iterator_t<R> code_unit_iter,
                     std::ranges::sentinel_t<R> code_unit_end)
      : code_unit_iter(std::move(code_unit_iter)), code_unit_end(std::move(code_unit_end)),
        codepoint(next()) {}
  constexpr int operator*() const noexcept {
    // -3..-1 are error codes, surrogates (U+D800..U+DFFF) are not allowed, and U+10FFFF is the
    // greatest valid codepoint.
    consteval_assert(-3 <= codepoint && codepoint < 0xD800 ||
                     0xE000 <= codepoint && codepoint < 0x11'0000);
    return codepoint;
  }
  constexpr iterator &operator++() {
    codepoint = next();
    return *this;
  }
  constexpr iterator operator++(int) {
    auto old = *this;
    ++*this;
    return old;
  }
  constexpr bool operator==(std::default_sentinel_t) const noexcept { return codepoint == eof; }
};

template <utf8_code_unit_sequence R> constexpr int codepoint_view<R>::iterator::next() {
  if (code_unit_iter == code_unit_end) {
    return eof;
  }
  const int a = *code_unit_iter++;
  if (a < 0x80) {
    // 00..7F
    // 0xxx'xxxx
    return a;
  }
  if (a < 0xC2 || 0xF4 < a) {
    return -3;
  }
  if (code_unit_iter == code_unit_end) {
    return -2;
  }
  const int b = *code_unit_iter ^ 0x80;
  if (b >> 6) {
    return -3;
  }
  ++code_unit_iter;
  int codepoint = a << 6 ^ b;
  if (a < 0xE0) {
    // C2..DF     80..BF
    // 110x'xxxx  10xx'xxxx
    return codepoint ^ (0xC0 << 6);
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

  if (code_unit_iter == code_unit_end) {
    return -2;
  }
  const int c = *code_unit_iter ^ 0x80;
  if (c >> 6) {
    return -3;
  }
  ++code_unit_iter;
  codepoint = (codepoint << 6) ^ c;
  if (a < 0xF0) {
    // E0         A0..BF     80..BF
    // E1..EC     80..BF     80..BF
    // ED         80..9F     80..BF
    // EE..EF     80..BF     80..BF
    // 1110'xxxx  10xx'xxxx  10xx'xxxx
    return codepoint ^ (0xE0 << 12);
  }

  if (code_unit_iter == code_unit_end) {
    return -2;
  }
  const int d = *code_unit_iter ^ 0x80;
  if (d >> 6) {
    return -3;
  }
  ++code_unit_iter;
  codepoint = (codepoint << 6) ^ d;
  // F0         90..BF     80..BF     80..BF
  // F1..F3     80..BF     80..BF     80..BF
  // F4         80..8F     80..BF     80..BF
  // 1111'0xxx  10xx'xxxx  10xx'xxxx  10xx'xxxx
  return codepoint ^ (0xF0 << 18);
}

constexpr inline struct to_codepoint : std::ranges::range_adaptor_closure<to_codepoint> {
  [[nodiscard]] constexpr auto operator()(utf8_code_unit_sequence auto &&code_units) const {
    return codepoint_view(code_units);
  }
} to_codepoint;
