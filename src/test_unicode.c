#include "unicode.h"
#include <assert.h>
#include <stddef.h>

int main()
{
	/* In UTF-8, the code point sequence <004D, 0430, 4E8C, 10302> is
	 * represented as <4D D0 B0 E4 BA 8C F0 90 8C 82>, where <4D>
	 * corresponds to U+004D, <D0 B0> corresponds to U+0430, <E4 BA 8C>
	 * corresponds to U+4E8C, and <F0 90 8C 82> corresponds to U+10302.
	 */
	const int code_points[] = {0x004D, 0x0430, 0x4E8C, 0x10302};
	const char8_t code_units[] =
	    u8"\x4D\xD0\xB0\xE4\xBA\x8C\xF0\x90\x8C\x82";
	struct utf8_iter it = {code_units, code_units + sizeof(code_units) - 1};
	assert(it.end - it.pos == 10);
	const size_t n = sizeof(code_points) / sizeof(int);
	assert(n == 4);
	for (size_t i = 0; i != n; ++i) {
		const int c = utf8_getc(&it);
		assert(c == code_points[i]);
	}
	assert(utf8_getc(&it) == -1);
}
