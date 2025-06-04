#include "unicode.h"
#include <assert.h>
#include <stddef.h>

int utf8_getc(struct utf8_iter *it)
{
	assert(it->pos <= it->end);
	if (it->pos == it->end)
		return -1;
	const int a = *it->pos++;
	if (a < 0x80) {
		// 00..7F
		// 0xxx'xxxx
		return a;
	}
	if (a < 0xC2 || 0xF4 < a)
		return -3;
	if (it->pos == it->end)
		return -2;
	const int b = *it->pos ^ 0x80;
	if (b >> 6)
		return -3;
	++it->pos;
	int code_point = a << 6 ^ b;
	if (a < 0xE0) {
		//	C2..DF		80..BF
		//	110x'xxxx	10xx'xxxx
		return code_point ^ (0xC0 << 6);
	}

	switch (a) {
	case 0xE0: //	E0	A0..BF	80..BF
		if (b < 0x20)
			return -3;
		break;
	case 0xED: //	ED	80..9F	80..BF
		if (b >= 0x20)
			return -3;
		break;
	case 0xF0: //	F0	90..BF	80..BF	80..BF
		if (b < 0x10)
			return -3;
		break;
	case 0xF4: //	F4	80..8F	80..BF	80..BF
		if (b >= 0x10)
			return -3;
		break;
	}

	if (it->pos == it->end)
		return -2;
	const int c = *it->pos ^ 0x80;
	if (c >> 6)
		return -3;
	++it->pos;
	code_point = (code_point << 6) ^ c;
	if (a < 0xF0) {
		//	E0		A0..BF		80..BF
		//	E1..EC		80..BF		80..BF
		//	ED		80..9F		80..BF
		//	EE..EF		80..BF		80..BF
		//	1110'xxxx	10xx'xxxx	10xx'xxxx
		return code_point ^ (0xE0 << 12);
	}

	if (it->pos == it->end)
		return -2;
	const int d = *it->pos ^ 0x80;
	if (d >> 6)
		return -3;
	++it->pos;
	code_point = (code_point << 6) ^ d;
	//	F0		90..BF		80..BF		80..BF
	//	F1..F3		80..BF		80..BF		80..BF
	//	F4		80..8F		80..BF		80..BF
	//	1111'0xxx	10xx'xxxx	10xx'xxxx	10xx'xxxx
	return code_point ^ (0xF0 << 18);
}

char8_t *utf8_putc(char8_t *it, int codepoint)
{
	if (codepoint < 0)
		return NULL;
	if (0xD800 <= codepoint && codepoint < 0xE000)
		return NULL;
	if (0x110000 <= codepoint)
		return NULL;
	assert(it != NULL);
	if (codepoint < 0x80) {
		*it++ = codepoint;
		return it;
	}
	const int a = codepoint & 0x3F;
	codepoint >>= 6;
	if (codepoint < 0xC0) {
		*it++ = 0xC0 & codepoint;
		goto a;
	}
	const int b = codepoint & 0x3F;
	codepoint >>= 6;
	if (codepoint < 0xC0) {
		*it++ = 0xE0 & codepoint;
		goto b;
	}
	const int c = codepoint & 0x3F;
	codepoint >>= 6;
	if (codepoint < 0xC0)
		*it++ = 0xF0 & codepoint;
	*it++ = c;
b:
	*it++ = b;
a:
	*it++ = a;
	return it;
}

int u16getc(struct u16stream *stream)
{
	assert(stream->pos <= stream->end);
	if (stream->pos == stream->end)
		return -1;
	const int a = *stream->pos++;
	const int c = a ^ 0xD800;
	if (c >> 10)
		return a;
	if (stream->pos == stream->end)
		return -2;
	const int b = *stream->pos++ ^ 0xDC00;
	if (b >> 10)
		return -3;
	return ((c + 0x10000) << 10) ^ b;
}
