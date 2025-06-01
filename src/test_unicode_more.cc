#include "unicode.h"
#include <cassert>
#include <string_view>
using namespace std::string_view_literals;

void assert_code_points(std::ranges::input_range auto &&code_points,
			std::u8string_view code_units)
{
	struct utf8_iter it = {code_units.begin(), code_units.end()};
	for (const int code_point : code_points) {
		assert(utf8_getc(&it) == code_point);
	}
	assert(utf8_getc(&it) == -1);
}

/** The UTF-8 code unit sequence <41 C3 B1 42> is well-formed, because it can be
 * partitioned into subsequences, all of which match the specification for UTF-8
 * in Table 3-7. It consists of the following minimal well-formed code unit
 * subsequences: <41>, <C3 B1>, and <42>.
 */
void well_formed_D86_01()
{
	assert_code_points(std::initializer_list{0x41, 0xF1, 0x42},
			   u8"\x41\xC3\xB1\x42"sv);
}

/** The UTF-8 code unit sequence <41 C2 C3 B1 42> is ill-formed, because it
 * contains one ill-formed subsequence. There is no subsequence for the C2 byte
 * which matches the specification for UTF-8 in Table 3-7. The code unit
 * sequence is partitioned into one minimal well-formed code unit subsequence,
 * <41>, followed by one ill-formed code unit subsequence, <C2>, followed by two
 * minimal well-formed code unit subsequences, <C3 B1> and <42>.
 */
void ill_formed_D86_02()
{
	assert_code_points(std::initializer_list{0x41, -3, 0xF1, 0x42},
			   u8"\x41\xC2\xC3\xB1\x42"sv);
}

/** In isolation, the UTF-8 code unit sequence <C2 C3> would be ill-formed, but
 * in the context of the UTF-8 code unit sequence <41 C2 C3 B1 42>, <C2 C3> does
 * not constitute an ill-formed code unit subsequence, because the C3 byte is
 * actually the first byte of the minimal well-formed UTF-8 code unit
 * subsequence <C3 B1>. Ill-formed code unit subsequences do not overlap with
 * minimal well-formed code unit subsequences. */
void ill_formed_D86_03()
{
	assert_code_points(std::initializer_list{-3, -2}, u8"\xC2\xC3"sv);
}

/* In UTF-8, the code point sequence <004D, 0430, 4E8C, 10302> is represented as
 * <4D D0 B0 E4 BA 8C F0 90 8C 82>, where <4D> corresponds to U+004D, <D0 B0>
 * corresponds to U+0430, <E4 BA 8C> corresponds to U+4E8C, and <F0 90 8C 82>
 * corresponds to U+10302.
 */
void well_formed_D92_01()
{
	assert_code_points(
	    std::initializer_list{0x004D, 0x0430, 0x4E8C, 0x10302},
	    u8"\x4D\xD0\xB0\xE4\xBA\x8C\xF0\x90\x8C\x82"sv);
}

/** The byte sequence <C0 AF> is ill-formed, because C0 is not well-formed in
 * the “First Byte” column.
 */
void ill_formed_first_byte()
{
	assert_code_points(std::initializer_list{-3, -3}, u8"\xC0\xAF"sv);
}

/** The byte sequence <E0 9F 80> is ill-formed, because in the row where E0 is
 * well-formed as a first byte, 9F is not well-formed as a second byte.
 */
void ill_formed_second_byte()
{
	assert_code_points(std::initializer_list{-3, -3}, u8"\xE0\x9F\x80"sv);
}

/** The byte sequence <F4 80 83 92> is well-formed, because every byte in that
 * sequence matches a byte range in a row of the table (the last row).
 */
void well_formed_four_bytes()
{
	assert_code_points(std::initializer_list{0x1000D2},
			   u8"\xF4\x80\x83\x92"sv);
}

void disallowed_byte_range(char8_t first_code_unit, char8_t last_code_unit)
{
	assert(first_code_unit <= last_code_unit);
	char8_t code_unit = first_code_unit;
	const char8_t *const begin = &code_unit;
	const char8_t *const end = begin + 1;
	for (; code_unit != last_code_unit; ++code_unit) {
		struct utf8_iter it = {begin, end};
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
	disallowed_byte_range(0xC0, 0xC1);
	disallowed_byte_range(0xF5, 0xFF);
}

int main()
{
	static_assert(sizeof(char8_t) == 1, "the size of char8_t must be 1");
	well_formed_D86_01();
	ill_formed_D86_02();
	ill_formed_D86_03();
	well_formed_D92_01();
	ill_formed_first_byte();
	ill_formed_second_byte();
	well_formed_four_bytes();
	disallowed_byte_values();
}