#include "unicode.hh"
#include <algorithm>
#include <string_view>

static_assert(std::ranges::input_range<codepoint_view<std::u8string_view>>);
static_assert(std::input_iterator<codepoint_view<std::u8string_view>::iterator>);
static_assert(codepoint_sequence<codepoint_view<std::u8string_view>>);

template <utf8_code_unit_sequence T, std::ranges::input_range U = std::initializer_list<int>>
constexpr bool test(T &&code_units, U &&codepoints) {
  return std::ranges::equal(code_units | to_codepoint, codepoints);
}

/* As a consequence of the well-formedness conditions specified in [Table
 * 3-7](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506), the following
 * byte values are disallowed in UTF-8: C0–C1, F5–FF.
 */
template <utf8_code_unit_sequence R> constexpr bool disallowed_byte_range(R &&code_units) {
  constexpr auto disallowed_byte = [](char8_t code_unit) {
    const char8_t singleton[] = {code_unit};
    return test(std::views::all(singleton), {-3});
  };
  return std::ranges::all_of(code_units | std::views::transform(disallowed_byte), std::identity{});
}

static_assert(disallowed_byte_range(std::views::iota(u8'\xC0', u8'\xC2')));
static_assert(disallowed_byte_range(std::views::iota(u8'\xF5', u8'\x00')));

using namespace std::string_view_literals;
static_assert(test(u8"hello world"sv, U"hello world"sv));

static_assert(test(u8"\x41\xC3\xB1\x42"sv, {0x41, 0xF1, 0x42}));
static_assert(test(u8"\x41\xC2\xC3\xB1\x42"sv, {0x41, -3, 0xF1, 0x42}));
static_assert(test(u8"\xC2\xC3"sv, {-3, -2}));
static_assert(test(u8"\x4D\xD0\xB0\xE4\xBA\x8C\xF0\x90\x8C\x82"sv,
                   {0x004D, 0x0430, 0x4E8C, 0x1'0302}));
static_assert(test(u8"\xC0\xAF"sv, {-3, -3}));
static_assert(test(u8"\xE0\x9F\x80"sv, {-3, -3}));
static_assert(test(u8"\xF4\x80\x83\x92"sv, {0x10'00D2}));
// rfc3629
static_assert(test(u8"\x41\xE2\x89\xA2\xCE\x91\x2E"sv, {0x0041, 0x2262, 0x0391, 0x002E}));
static_assert(test(u8"\xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4"sv, {0xD55C, 0xAD6D, 0xC5B4}));
static_assert(test(u8"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"sv, {0x65E5, 0x672C, 0x8A9E}));
static_assert(test(u8"\xEF\xBB\xBF\xF0\xA3\x8E\xB4"sv, {0xFEFF, 0x2'33B4}));

int main() {}
