#include "json.h"
#include <assert.h>
#include <stddef.h>

static int skip_whitespace(struct utf8_iter *source, int c)
{
	for (;; c = utf8_getc(source)) {
		switch (c) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		default:
			// Loop termination proof: `c` will eventually be EOF, a
			// UTF-8 encoding error, or not a whitespace.
			return c;
		}
	}
}

/** The byte sequence `literal`..`literal + literal_size - 1` MUST be a
well-formed UTF-8 string.

Returns -5 if source and literal doesn't match.

Otherwise, the return value is that of `utf8_getc`.
*/
static int lex_literal(struct utf8_iter *source, const char8_t *literal,
		       size_t literal_size)
{
	struct utf8_iter match = {literal, literal + literal_size};
	while (1) {
		const int d = utf8_getc(&match);
		assert(d >= -1);
		const int c = utf8_getc(source);
		// Loop termination proof: `d` will eventually be the null
		// character regardless of what value `c` takes on.
		if (d == -1 || c < 0)
			return c;
		if (c != d)
			return -5;
	}
}

static int lex_string(struct utf8_iter *source)
{
	while (1) {
		const int c = utf8_getc(source);
		if (c < 0)
			return c;
		switch (c) {
		case '"':
			return utf8_getc(source);
		case '\\': {
			const int c = utf8_getc(source);
			if (c < 0)
				return c;
			switch (c) {
			case '"':
			case '\\':
			case '/':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
				break;
			case 'u':
				for (int i = 0; i != 4; ++i) {
					const int c = utf8_getc(source);
					if (c < 0)
						return c;
					if ('0' <= c && c <= '9' ||
					    'a' <= c && c <= 'f' ||
					    'A' <= c && c <= 'F')
						continue;
					return -4; // FIXME: another error code
				}
				break;
			default:
				return -4; // FIXME: another error code
			}
			break;
		}
		default:
			if (0x20 <= c && c <= 0x21 || 0x23 <= c && c <= 0x5B ||
			    0x5D <= c && c <= 0x10FFFF)
				break;
			return -4; // FIXME: another error code
		}
	}
}

static int parse_value(struct utf8_iter *source, int c);

static int parse_array(struct utf8_iter *source)
{
	for (int c = utf8_getc(source);;) {
		c = parse_value(source, c);
		if (c < 0)
			return c;

		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case ']':
			return utf8_getc(source);
		case ',':
			c = utf8_getc(source);
			break;
		default:
			return -4; // FIXME: another error code
		}
	}
}

static int parse_object(struct utf8_iter *source)
{
	for (int c = utf8_getc(source);;) {
		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case '"':
			c = lex_string(source);
			break;
		default:
			return -4; // FIXME: another error code
		}

		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case ':':
			c = utf8_getc(source);
			break;
		default:
			return -4; // FIXME: another error code
		}

		c = parse_value(source, c);
		if (c < 0)
			return c;

		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case '}':
			return utf8_getc(source);
		case ',':
			c = utf8_getc(source);
			break;
		default:
			return -4; // FIXME: another error code
		}
	}
}

static int parse_value(struct utf8_iter *source, int c)
{
	c = skip_whitespace(source, c);
	if (c < 0)
		return c;
	switch (c) {
	case 'f':
		return lex_literal(source, u8"alse", 4);
	case 'n':
		return lex_literal(source, u8"ull", 3);
	case 't':
		return lex_literal(source, u8"rue", 3);
	case '"':
		return lex_string(source);
	case '[':
		return parse_array(source);
	case '{':
		return parse_object(source);
	}
	return -4; // FIXME: another error code
}

int json_parse(struct utf8_iter *source)
{
	// FIXME: BOM?
	const int c = parse_value(source, utf8_getc(source));
	return skip_whitespace(source, c);
}
