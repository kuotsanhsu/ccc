#include "unicode.hh"
#include <algorithm>
#include <cassert>
#include <string_view>

static_assert(std::input_iterator<u8iterator<std::u8string_view>>);

template <typename R> constexpr u8iterator<R> next(u8iterator<R> it) { return ++it; }

int main() {
  using namespace std::string_view_literals;
  constexpr u8iterator h(u8"hello world"sv);
  static_assert(*h == 'h');
  constexpr auto e = next(h);
  static_assert(*e == 'e');
  constexpr auto l = next(e);
  static_assert(*l == 'l');
  constexpr auto o = next(next(l));
  for (auto it = o; const auto c : u8"o world"sv) {
    assert(*it++ == c);
  }

  /* As a consequence of the well-formedness conditions specified in [Table
   * 3-7](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506), the following
   * byte values are disallowed in UTF-8: C0–C1, F5–FF.
   */
  for (const auto code_units : {
           std::ranges::iota_view(u8'\xC0', u8'\xC2'),
           std::ranges::iota_view(u8'\xF5', u8'\x00'),
       }) {
    for (const auto code_unit : code_units) {
      const auto begin = &code_unit;
      u8iterator it(std::ranges::subrange(begin, std::next(begin)));
      assert(*it++ == -3);
      assert(*it == it.sentinal);
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
    const u8iterator it(code_units);
    assert(std::ranges::equal(std::ranges::subrange(it, it.sentinal), codepoints));
  }
}
