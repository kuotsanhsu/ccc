#pragma once
#if __SIZEOF_INT__ != 4
#error "size of int MUST be 4"
#endif

#ifdef __cplusplus
extern "C" {
#ifndef __cpp_char8_t
typedef unsigned char char8_t;
#endif
#elif __has_include(<uchar.h>)
#include <uchar.h>
#else
typedef unsigned char char8_t;
#endif

struct utf8_iter {
	/** `pos <= end`. */
	const char8_t *pos;
	/** MUST NOT derefence `end`. */
	const char8_t *const end;
};

/** Gets a
 * [UTF-8](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506)
 * code point from the buffer. Returns a code point at most U+10FFFF upon
 * success.
 *
 * Returns -1 if the buffer is empty, i.e. EOF. Further calls to this function
 * with the same argument is safe and will always return -1.
 * Postcondition: `it->pos == it->end`.
 *
 * Returns -2 if EOF is reached prematurely, i.e. expects more code units; never
 * happens to ASCII. NOTE: further calls to this function with the same argument
 * will always return -1 thereby hiding the fact that the EOF is premature.
 * Postcondition: `it->pos == it->end`.
 *
 * Returns -3 if an invalid code unit is encountered before reaching EOF, and
 * `it->pos` will point at the immediate next code unit. NOTE: further calls to
 * this function with the same argument will start parsing a new code point
 * starting at the code unit immediately after the encountered invalid code unit
 * thereby hiding the fact that the UTF-8 string is ill-formed.
 * Postcondition: `it->pos <= it->end`.
 */
int utf8_getc(struct utf8_iter *it);

#ifdef __cplusplus
}
#endif
