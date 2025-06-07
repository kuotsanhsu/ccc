#include "json_syntax.h"
#include <assert.h>

struct codepoint_stream {
	const char8_t *pos;
	const char8_t *const end;
};

int get_codepoint(struct codepoint_stream *it)
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

struct codepoint_stream *utf8_stream(const char8_t *string, size_t string_size)
{
	struct codepoint_stream stream = {string, string + string_size};
	return &stream; // FIXME: dangling pointer
}

void handle_json_string(struct codepoint_stream *source)
{
	__builtin_debugtrap();
}
