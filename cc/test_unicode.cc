#include <algorithm>
#include <cassert>
#include <ranges>
#include <string_view>

template <typename R, typename CharT>
concept code_unit_sequence =
    std::ranges::input_range<R> && std::same_as<std::ranges::range_value_t<R>, CharT>;

template <typename R>
concept utf8_code_unit_sequence = code_unit_sequence<R, char8_t>;

template <utf8_code_unit_sequence R>
class codepoint_view : public std::ranges::view_interface<codepoint_view<R>> {
  R code_units;

public:
  class iterator;
  constexpr codepoint_view(R code_units) noexcept : code_units(std::move(code_units)) {}
  constexpr iterator begin() const {
    return {std::ranges::begin(code_units), std::ranges::end(code_units)};
  }
  constexpr std::default_sentinel_t end() const noexcept { return std::default_sentinel; }
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
  constexpr int operator*() const noexcept { return codepoint; }
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
  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, char8_t>
  [[nodiscard]] constexpr auto operator()(R &&code_units) const {
    return codepoint_view(std::forward<R>(code_units));
  }
} to_codepoint;

static_assert(std::ranges::input_range<codepoint_view<std::u8string_view>>);
static_assert(std::input_iterator<codepoint_view<std::u8string_view>::iterator>);

int main() {
  using namespace std::string_view_literals;
  static_assert(std::ranges::equal(u8"hello world"sv | to_codepoint, U"hello world"sv));

  /* As a consequence of the well-formedness conditions specified in [Table
   * 3-7](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506), the following
   * byte values are disallowed in UTF-8: C0–C1, F5–FF.
   */
  for (const auto code_units : {
           std::ranges::iota_view(u8'\xC0', u8'\xC2'),
           std::ranges::iota_view(u8'\xF5', u8'\x00'),
       }) {
    for (const auto code_unit : code_units) {
      const char8_t singleton[] = {code_unit};
      constexpr int bad[] = {-3};
      assert(std::ranges::equal(std::views::all(singleton) | to_codepoint, bad));
    }
  }

  for (const auto &[code_units, codepoints] :
       std::initializer_list<std::pair<std::u8string_view, std::initializer_list<int>>>{
           {u8"\x41\xC3\xB1\x42", {0x41, 0xF1, 0x42}},
           {u8"\x41\xC2\xC3\xB1\x42", {0x41, -3, 0xF1, 0x42}},
           {u8"\xC2\xC3", {-3, -2}},
           {u8"\x4D\xD0\xB0\xE4\xBA\x8C\xF0\x90\x8C\x82", {0x004D, 0x0430, 0x4E8C, 0x1'0302}},
           {u8"\xC0\xAF", {-3, -3}},
           {u8"\xE0\x9F\x80", {-3, -3}},
           {u8"\xF4\x80\x83\x92", {0x10'00D2}},
           // rfc3629
           {u8"\x41\xE2\x89\xA2\xCE\x91\x2E", {0x0041, 0x2262, 0x0391, 0x002E}},
           {u8"\xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4", {0xD55C, 0xAD6D, 0xC5B4}},
           {u8"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E", {0x65E5, 0x672C, 0x8A9E}},
           {u8"\xEF\xBB\xBF\xF0\xA3\x8E\xB4", {0xFEFF, 0x2'33B4}},
       }) {
    assert(std::ranges::equal(code_units | to_codepoint, codepoints));
  }
}
