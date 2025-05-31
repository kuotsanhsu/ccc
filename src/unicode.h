#pragma once
#if __SIZEOF_INT__ != 4
#error "size of int MUST be 4"
#endif
#if __STDC_VERSION__ >= 202311L
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

/** Gets a [UTF-8](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506) code
 * point from the buffer. Returns a code point at most U+10FFFF upon success.
 *
 * Returns -1 if the buffer is empty, i.e. EOF. `pos == end`.
 *
 * Returns -2 if EOF is reached prematurely, i.e. expects more code units; never happens to ASCII.
 * `pos == end`.
 *
 * Returns -3 if an invalid code unit is encountered before reaching EOF, and `pos` will stop
 * incrementing at this point. `pos < end`.
 */
int utf8_getc(struct utf8_iter *it);
