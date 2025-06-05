#include <assert.h>
#include <ctype.h>
#include <stddef.h>

typedef unsigned char char8_t;
struct codepoint_stream;
int get_codepoint(struct codepoint_stream *);
struct codepoint_stream *utf8_stream(const char8_t *, size_t);

static int lex_whitespace(struct codepoint_stream *source, int c)
{
	for (;; c = get_codepoint(source)) {
		switch (c) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		default:
			return c;
		}
	}
}

static int lex_literal(struct codepoint_stream *source,
		       struct codepoint_stream *literal)
{
	while (1) {
		const int d = get_codepoint(literal);
		assert(d >= -1);
		const int c = get_codepoint(source);
		if (d == -1 || c < 0)
			return c;
		if (c != d)
			return -5; // FIXME: another error code
	}
}

/**
 * Returns -4 if a consecutive sequence of 4 hex digits cannot be parsed.
 *
 * Returns -3..-1 if utf8_getc reports error. The return value is
 * exactly the error value returned by utf8_getc.
 *
 * Returns 0 if successful.
 */
static int lex_4_xdigits(struct codepoint_stream *source)
{
	for (int n = 4; n--;) {
		const int c = get_codepoint(source);
		if (c < 0)
			return c;
		if (isxdigit(c))
			continue;
		return -4; // FIXME: another error code
	}
	return 0;
}

static int lex_string(struct codepoint_stream *source)
{
	while (1) {
		const int c = get_codepoint(source);
		if (c < 0)
			return c;
		switch (c) {
		case '"':
			return get_codepoint(source);
		case '\\': {
			const int c = get_codepoint(source);
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
			case 'u': {
				const int ret = lex_4_xdigits(source);
				if (ret < 0)
					return ret;
				break;
			}
			default:
				return -4; // FIXME: another error code
			}
			break;
		}
		default:
			// Control characters (U+0000 through U+001F) MUST be
			// escaped.
			if (c < 0x20)
				return -4; // FIXME: another error code
			// Quotation mark (U+22) WILL NOT appear here.
			assert(c != 0x22);
			// Reverse solidus (U+5C) WILL NOT appear here.
			assert(c != 0x5C);
			// WILL be a valid code point.
			assert(c < 0x110000);
		}
	}
}

static int lex_1_or_more_digits(struct codepoint_stream *source, int c)
{
	if (c < 0)
		return c;
	if (!isdigit(c))
		return -4; // FIXME: another error code
	do {
		c = get_codepoint(source);
	} while (isdigit(c));
	return c;
}

static int lex_number_frac_exp(struct codepoint_stream *source, int c)
{
	if (c == '.')
		c = lex_1_or_more_digits(source, get_codepoint(source));
	switch (c) {
	case 'e':
	case 'E':
		c = get_codepoint(source);
		switch (c) {
		case '-':
		case '+':
			c = get_codepoint(source);
		}
		c = lex_1_or_more_digits(source, c);
	}
	return c;
}

static int lex_number_after_first_digit(struct codepoint_stream *source)
{
	int c = get_codepoint(source);
	while (isdigit(c)) {
		c = get_codepoint(source);
	}
	return lex_number_frac_exp(source, c);
}

static int lex_number_int_frac_exp(struct codepoint_stream *source)
{
	const int c = get_codepoint(source);
	if (c < 0)
		return c;
	if ('1' <= c && c <= '9')
		return lex_number_after_first_digit(source);
	if (c == '0')
		return lex_number_frac_exp(source, get_codepoint(source));
	return -4; // FIXME: another error code
}

static int lex_value(struct codepoint_stream *source, int c);

static int lex_array(struct codepoint_stream *source)
{
	for (int c = get_codepoint(source);;) {
		c = lex_value(source, c);
		if (c < 0)
			return c;
		c = lex_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case ']':
			return get_codepoint(source);
		case ',':
			c = get_codepoint(source);
			break;
		default:
			return -4; // FIXME: another error code
		}
	}
}

static int lex_object(struct codepoint_stream *source)
{
	for (int c = get_codepoint(source);;) {
		c = lex_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case '"':
			c = lex_string(source);
			break;
		default:
			return -4; // FIXME: another error code
		}

		c = lex_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case ':':
			c = get_codepoint(source);
			break;
		default:
			return -4; // FIXME: another error code
		}

		c = lex_value(source, c);
		if (c < 0)
			return c;

		c = lex_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case '}':
			return get_codepoint(source);
		case ',':
			c = get_codepoint(source);
			break;
		default:
			return -4; // FIXME: another error code
		}
	}
}

static int lex_value(struct codepoint_stream *source, int c)
{
	c = lex_whitespace(source, c);
	if (c < 0)
		return c;
	if ('1' <= c && c <= '9')
		return lex_number_after_first_digit(source);
	switch (c) {
	case '0':
		return lex_number_frac_exp(source, get_codepoint(source));
	case '-':
		return lex_number_int_frac_exp(source);
	case 'f':
		return lex_literal(source, utf8_stream(u8"alse", 4));
	case 'n':
		return lex_literal(source, utf8_stream(u8"ull", 3));
	case 't':
		return lex_literal(source, utf8_stream(u8"rue", 3));
	case '"':
		return lex_string(source);
	case '[':
		return lex_array(source);
	case '{':
		return lex_object(source);
	}
	return -4; // FIXME: another error code
}

static int lex_json_text(struct codepoint_stream *source)
{
	// FIXME: BOM?
	return lex_whitespace(source, lex_value(source, get_codepoint(source)));
}
