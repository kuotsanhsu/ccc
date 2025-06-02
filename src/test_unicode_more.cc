#include "unicode.h"
#include <cassert>
#include <ranges>
#include <string_view>

#ifdef assert
#undef assert
#define assert(e) (__builtin_expect(!(e), 0) ? __builtin_trap() : (void)0)
#endif

void disallowed_byte_range(std::ranges::range auto &&code_units)
{
	for (const char8_t code_unit : code_units) {
		const auto begin = &code_unit;
		const auto end = std::next(begin);
		utf8_iter it{begin, end};
		assert(utf8_getc(&it) == -3);
		assert(it.pos == end);
	}
}

/** As a consequence of the well-formedness conditions specified in [Table
 * 3-7](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506),
 * the following byte values are disallowed in UTF-8: C0–C1, F5–FF.
 */
void disallowed_byte_values()
{
	disallowed_byte_range(std::ranges::iota_view(0xC0, 0xC2));
	disallowed_byte_range(std::ranges::iota_view(0xF5, 0x100));
}

int main()
{
	disallowed_byte_values();
	for (const auto &[code_units, code_points] : std::initializer_list<
		 std::pair<std::u8string_view, std::initializer_list<int>>>{
		 {u8"\x41\xC3\xB1\x42", {0x41, 0xF1, 0x42}},
		 {u8"\x41\xC2\xC3\xB1\x42", {0x41, -3, 0xF1, 0x42}},
		 {u8"\xC2\xC3", {-3, -2}},
		 {u8"\x4D\xD0\xB0\xE4\xBA\x8C\xF0\x90\x8C\x82",
		  {0x004D, 0x0430, 0x4E8C, 0x10302}},
		 {u8"\xC0\xAF", {-3, -3}},
		 {u8"\xE0\x9F\x80", {-3, -3}},
		 {u8"\xF4\x80\x83\x92", {0x1000D2}},
		 // rfc3629
		 {u8"\x41\xE2\x89\xA2\xCE\x91\x2E",
		  {0x0041, 0x2262, 0x0391, 0x002E}},
		 {u8"\xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4",
		  {0xD55C, 0xAD6D, 0xC5B4}},
		 {u8"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E",
		  {0x65E5, 0x672C, 0x8A9E}},
		 {u8"\xEF\xBB\xBF\xF0\xA3\x8E\xB4", {0xFEFF, 0x233B4}},
	     }) {
		utf8_iter it{code_units.begin(), code_units.end()};
		for (const int code_point : code_points) {
			assert(utf8_getc(&it) == code_point);
		}
		assert(utf8_getc(&it) == -1);
	}
}
