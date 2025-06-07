#include <cassert>
#include <cctype>

struct utf8_iter {
  /** `pos <= end`. */
  const char8_t *pos;
  /** MUST NOT derefence `end`. */
  const char8_t *const end;
};

struct u16stream {
  /** `pos <= end`. */
  const char16_t *pos;
  /** MUST NOT derefence `end`. */
  const char16_t *const end;
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

/** FIXME: unsafe */
char8_t *utf8_putc(char8_t *it, int codepoint);

/** Decode the high code unit or the only code unit of a UTF-16 code point.
 *
 * Returns 0 if code_unit is itself a valid UTF-16 code point.
 *
 * Returns non-zero if a low code unit is to be expected. In this case, the
 * return value can be XORed with the return value of u16low.
 */
int u16high(char16_t code_unit);

// FIXME: bad name? don't pass in high?
int u16low(int high, char16_t code_unit);

int u16getc(struct u16stream *stream);

struct json {
  char8_t *pos;
};

enum class json_marker : char8_t {
  JSON_begin_array = '[',
  JSON_begin_object = '{',
  JSON_end_array = ']',
  JSON_end_object = '}',
  JSON_name_separator = ':',
  JSON_value_separator = ',',
  JSON_false = 'f',
  JSON_null = 'n',
  JSON_true = 't',
  JSON_e = '/', // -./0123456789
  JSON_begin_string = '"',
  JSON_end_string = u8'\xC0',
};

int utf8_getc(struct utf8_iter *it) {
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

char8_t *utf8_putc(char8_t *it, int codepoint) {
  if (codepoint < 0)
    return nullptr;
  if (0xD800 <= codepoint && codepoint < 0xE000)
    return nullptr;
  if (0x110000 <= codepoint)
    return nullptr;
  assert(it != nullptr);
  if (codepoint < 0x80) {
    *it++ = codepoint;
    return it;
  }
  const int a = codepoint & 0x3F;
  if ((codepoint >>= 6) < 0xC0) {
    *it++ = 0xC0 & codepoint;
  } else {
    const int b = codepoint & 0x3F;
    if ((codepoint >>= 6) < 0xC0) {
      *it++ = 0xE0 & codepoint;
    } else {
      const int c = codepoint & 0x3F;
      if ((codepoint >>= 6) < 0xC0) {
        *it++ = 0xF0 & codepoint;
      }
      *it++ = c;
    }
    *it++ = b;
  }
  *it++ = a;
  return it;
}

int u16high(char16_t code_unit) {
  const int c = code_unit ^ 0xD800;
  if (c >> 10)
    return 0;
  return (c + 0x10000) << 10;
}

int u16low(int high, char16_t code_unit) {
  assert(0x10000 <= high && high < 0x110000);
  const int low = code_unit ^ 0xDC00;
  if (low >> 10)
    return -3;
  return high ^ low;
}

int u16getc(struct u16stream *stream) {
  assert(stream->pos <= stream->end);
  if (stream->pos == stream->end)
    return -1;
  const int a = *stream->pos++;
  const int high = u16high(a);
  if (!high)
    return a;
  if (stream->pos == stream->end)
    return -2;
  return u16low(high, *stream->pos++);
}

int json_parse(struct json *json, struct utf8_iter *source);

struct codepoint_stream;
int get_codepoint(struct codepoint_stream *);
struct codepoint_stream *utf8_stream(const char8_t *, size_t);

enum json_error {
  JSON_ERR_lex_value = -10,
  JSON_ERR_lex_literal = -11,
  JSON_ERR_lex_object_member = -12,
  JSON_ERR_lex_object_name_separator = -13,
  JSON_ERR_lex_end_object_or_value_separator = -14,
  JSON_ERR_lex_end_array_or_value_separator = -15,
  JSON_ERR_lex_xdigit = -16,
  JSON_ERR_lex_digit = -17,
  JSON_ERR_lex_number = -18,
  JSON_ERR_lex_escape = -19,
  JSON_ERR_lex_string = -20,
};

int lex_json_text(struct codepoint_stream *source);

void handle_json_whitespace(struct codepoint_stream *source);

void handle_json_string(struct codepoint_stream *source);

void handle_json_array(struct codepoint_stream *source);

void handle_json_object(struct codepoint_stream *source);

void handle_json_literal(struct codepoint_stream *source);

static int lex_whitespace(struct codepoint_stream *source, int c) {
  for (;; c = get_codepoint(source)) {
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      break;
    default:
      handle_json_whitespace(source);
      return c;
    }
  }
}

static int lex_literal(struct codepoint_stream *source, struct codepoint_stream *literal) {
  while (1) {
    const int d = get_codepoint(literal);
    assert(d >= -1);
    const int c = get_codepoint(source);
    if (c < 0)
      return c;
    if (d == -1) {
      handle_json_literal(source);
      return c;
    }
    if (c != d)
      return JSON_ERR_lex_literal;
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
static int lex_4_xdigits(struct codepoint_stream *source) {
  for (int n = 4; n--;) {
    const int c = get_codepoint(source);
    if (c < 0)
      return c;
    if (isxdigit(c))
      continue;
    return JSON_ERR_lex_xdigit;
  }
  return 0;
}

static int lex_string(struct codepoint_stream *source) {
  while (1) {
    const int c = get_codepoint(source);
    if (c < 0)
      return c;
    switch (c) {
    case '"':
      handle_json_string(source);
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
        return JSON_ERR_lex_escape;
      }
      break;
    }
    default:
      // Control characters (U+0000 through U+001F) MUST be
      // escaped.
      if (c < 0x20)
        return JSON_ERR_lex_string;
      // Quotation mark (U+22) WILL NOT appear here.
      assert(c != 0x22);
      // Reverse solidus (U+5C) WILL NOT appear here.
      assert(c != 0x5C);
      // WILL be a valid code point.
      assert(c < 0x110000);
    }
  }
}

static int lex_1_or_more_digits(struct codepoint_stream *source, int c) {
  if (c < 0)
    return c;
  if (!isdigit(c))
    return JSON_ERR_lex_digit;
  do {
    c = get_codepoint(source);
  } while (isdigit(c));
  return c;
}

static int lex_number_frac_exp(struct codepoint_stream *source, int c) {
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

static int lex_number_after_first_digit(struct codepoint_stream *source) {
  int c = get_codepoint(source);
  while (isdigit(c)) {
    c = get_codepoint(source);
  }
  return lex_number_frac_exp(source, c);
}

static int lex_number_int_frac_exp(struct codepoint_stream *source) {
  const int c = get_codepoint(source);
  if (c < 0)
    return c;
  if ('1' <= c && c <= '9')
    return lex_number_after_first_digit(source);
  if (c == '0')
    return lex_number_frac_exp(source, get_codepoint(source));
  return JSON_ERR_lex_escape;
}

static int lex_value(struct codepoint_stream *source, int c);

static int lex_array(struct codepoint_stream *source) {
  for (int c = get_codepoint(source);;) {
    c = lex_value(source, c);
    if (c < 0)
      return c;
    c = lex_whitespace(source, c);
    if (c < 0)
      return c;
    switch (c) {
    case ']':
      handle_json_array(source);
      return get_codepoint(source);
    case ',':
      c = get_codepoint(source);
      break;
    default:
      return JSON_ERR_lex_end_array_or_value_separator;
    }
  }
}

static int lex_object(struct codepoint_stream *source) {
  for (int c = get_codepoint(source);;) {
    c = lex_whitespace(source, c);
    if (c < 0)
      return c;
    switch (c) {
    case '"':
      c = lex_string(source);
      break;
    default:
      return JSON_ERR_lex_object_member;
    }

    c = lex_whitespace(source, c);
    if (c < 0)
      return c;
    switch (c) {
    case ':':
      c = get_codepoint(source);
      break;
    default:
      return JSON_ERR_lex_object_name_separator;
    }

    c = lex_value(source, c);
    if (c < 0)
      return c;

    c = lex_whitespace(source, c);
    if (c < 0)
      return c;
    switch (c) {
    case '}':
      handle_json_object(source);
      return get_codepoint(source);
    case ',':
      c = get_codepoint(source);
      break;
    default:
      return JSON_ERR_lex_end_object_or_value_separator;
    }
  }
}

static int lex_value(struct codepoint_stream *source, int c) {
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
  return JSON_ERR_lex_value;
}

int lex_json_text(struct codepoint_stream *source) {
  int c = get_codepoint(source);
  if (c == 0xFEFF) // Skip BOM.
    c = get_codepoint(source);
  return lex_whitespace(source, lex_value(source, c));
}
