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

static int lex_string(struct json *json, struct utf8_iter *source)
{
	*json->pos++ = JSON_begin_string;
	while (1) {
		const int c = utf8_getc(source);
		if (c < 0)
			return c;
		switch (c) {
		case '"':
			*json->pos++ = JSON_end_string;
			return utf8_getc(source);
		case '\\': {
			const int c = utf8_getc(source);
			if (c < 0)
				return c;
			switch (c) {
			case '"':
			case '\\':
			case '/':
				*json->pos++ = c;
				break;
			case 'b':
				*json->pos++ = '\b';
				break;
			case 'f':
				*json->pos++ = '\f';
				break;
			case 'n':
				*json->pos++ = '\n';
				break;
			case 'r':
				*json->pos++ = '\r';
				break;
			case 't':
				*json->pos++ = '\t';
				break;
			case 'u': {
				char16_t code_unit = 0;
				for (int i = 0; i != 4; ++i) {
					int c = utf8_getc(source);
					if (c < 0)
						return c;
					if ('0' <= c && c <= '9')
						c -= '0';
					else if ('a' <= c && c <= 'f')
						c = c - 'a' + 10;
					else if ('A' <= c && c <= 'F')
						c = c - 'A' + 10;
					else
						return -4; // FIXME: another
							   // error code
					code_unit = (code_unit << 4) ^ c;
				}
				// TODO: convert `code_unit` to UTF-8
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
			// FIXME: json->pos COULD overflow in utf8_putc
			json->pos = utf8_putc(json->pos, c);
		}
	}
}

static int lex_1_or_more_digits(struct json *json, struct utf8_iter *source,
				int c)
{
	if (c < 0)
		return c;
	if (c < '0' || '9' < c)
		return -4; // FIXME: another error code
	do {
		*json->pos = c;
		c = utf8_getc(source);
	} while ('0' <= c && c <= '9');
	return c;
}

// number = [ minus ] int [ frac ] [ exp ]
//                          ^^^^     ^^^
static int lex_number_frac_exp(struct json *json, struct utf8_iter *source,
			       int c)
{
	// number = [ minus ] int [ frac ] [ exp ]
	//                          ^^^^
	//                          frac = decimal-point 1*DIGIT
	if (c == '.') { // decimal-point = %x2E ; .
		*json->pos++ = c;
		c = lex_1_or_more_digits(json, source, utf8_getc(source));
	}
	// number = [ minus ] int [ frac ] [ exp ]
	//                                   ^^^
	//                                   exp = e [ minus / plus ] 1*DIGIT
	switch (c) {
	case 'e': // e = %x65 / %x45 ; e E
	case 'E':
		*json->pos++ = JSON_e;
		c = utf8_getc(source);
		switch (c) {
		case '-': // minus = %x2D ; -
			*json->pos++ = c;
			[[fallthrough]];
		case '+': // plus = %x2B  ; +
			c = utf8_getc(source);
		}
		c = lex_1_or_more_digits(json, source, c);
	}
	return c;
}

// number = [ minus ] int [ frac ] [ exp ]
//                    ^^^   ^^^^     ^^^
//                    int = zero / ( digit1-9 *DIGIT )
//                                            ^^^^^^
static int lex_number_after_first_digit(struct json *json,
					struct utf8_iter *source)
{
	int c = utf8_getc(source);
	while ('0' <= c && c <= '9') {
		*json->pos++ = c;
		c = utf8_getc(source);
	}
	return lex_number_frac_exp(json, source, c);
}

// number = [ minus ] int [ frac ] [ exp ]
//                    ^^^   ^^^^     ^^^
static int lex_number_int_frac_exp(struct json *json, struct utf8_iter *source)
{
	const int c = utf8_getc(source);
	if (c < 0)
		return c;
	// int = zero / ( digit1-9 *DIGIT )
	//                ^^^^^^^^
	if ('1' <= c && c <= '9') {
		*json->pos++ = c;
		return lex_number_after_first_digit(json, source);
	}
	// int = zero / ( digit1-9 *DIGIT )
	//       ^^^^
	if (c == '0') {
		*json->pos++ = c;
		return lex_number_frac_exp(json, source, utf8_getc(source));
	}
	return -4; // FIXME: another error code
}

static int parse_value(struct json *json, struct utf8_iter *source, int c);

static int parse_array(struct json *json, struct utf8_iter *source)
{
	*json->pos++ = JSON_begin_array;
	for (int c = utf8_getc(source);;) {
		c = parse_value(json, source, c);
		if (c < 0)
			return c;

		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case ']':
			*json->pos++ = JSON_end_array;
			return utf8_getc(source);
		case ',':
			*json->pos++ = JSON_value_separator;
			c = utf8_getc(source);
			break;
		default:
			return -4; // FIXME: another error code
		}
	}
}

static int parse_object(struct json *json, struct utf8_iter *source)
{
	*json->pos++ = JSON_begin_object;
	for (int c = utf8_getc(source);;) {
		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case '"':
			c = lex_string(json, source);
			break;
		default:
			return -4; // FIXME: another error code
		}

		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case ':':
			*json->pos++ = JSON_name_separator;
			c = utf8_getc(source);
			break;
		default:
			return -4; // FIXME: another error code
		}

		c = parse_value(json, source, c);
		if (c < 0)
			return c;

		c = skip_whitespace(source, c);
		if (c < 0)
			return c;
		switch (c) {
		case '}':
			*json->pos++ = JSON_end_object;
			return utf8_getc(source);
		case ',':
			*json->pos++ = JSON_value_separator;
			c = utf8_getc(source);
			break;
		default:
			return -4; // FIXME: another error code
		}
	}
}

static int parse_value(struct json *json, struct utf8_iter *source, int c)
{
	c = skip_whitespace(source, c);
	if (c < 0)
		return c;
	if ('1' <= c && c <= '9') {
		*json->pos++ = c;
		return lex_number_after_first_digit(json, source);
	}
	switch (c) {
	case '0':
		*json->pos++ = c;
		return lex_number_frac_exp(json, source, utf8_getc(source));
	case '-':
		*json->pos++ = c;
		return lex_number_int_frac_exp(json, source);
	case 'f':
		*json->pos++ = JSON_false;
		return lex_literal(source, u8"alse", 4);
	case 'n':
		*json->pos++ = JSON_null;
		return lex_literal(source, u8"ull", 3);
	case 't':
		*json->pos++ = JSON_true;
		return lex_literal(source, u8"rue", 3);
	case '"':
		return lex_string(json, source);
	case '[':
		return parse_array(json, source);
	case '{':
		return parse_object(json, source);
	}
	return -4; // FIXME: another error code
}

int json_parse(struct json *json, struct utf8_iter *source)
{
	// FIXME: BOM?
	const int c = parse_value(json, source, utf8_getc(source));
	return skip_whitespace(source, c);
}
