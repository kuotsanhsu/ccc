#include "unicode.hh"
#include <cassert>
#include <string_view>

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

template <typename R> class json_parser {
  u8iterator<R> source;

public:
  json_parser(u8iterator<R> source) : source(source) {}

  int lex_json_text() {
    int c = get_codepoint(source);
    if (c == 0xFEFF) // Skip BOM.
      c = get_codepoint(source);
    return lex_whitespace(source, lex_value(source, c));
  }

private:
  int lex_whitespace(int c) {
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

  int lex_literal(const u8iterator<R> &literal) {
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
  int lex_4_xdigits() {
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

  int lex_string() {
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

  int lex_1_or_more_digits(int c) {
    if (c < 0)
      return c;
    if (!isdigit(c))
      return JSON_ERR_lex_digit;
    do {
      c = get_codepoint(source);
    } while (isdigit(c));
    return c;
  }

  int lex_number_frac_exp(int c) {
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

  int lex_number_after_first_digit() {
    int c = get_codepoint(source);
    while (isdigit(c)) {
      c = get_codepoint(source);
    }
    return lex_number_frac_exp(source, c);
  }

  int lex_number_int_frac_exp() {
    const int c = get_codepoint(source);
    if (c < 0)
      return c;
    if ('1' <= c && c <= '9')
      return lex_number_after_first_digit(source);
    if (c == '0')
      return lex_number_frac_exp(source, get_codepoint(source));
    return JSON_ERR_lex_escape;
  }

  int lex_array() {
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

  int lex_object() {
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

  int lex_value(int c) {
    using namespace std::string_view_literals;
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
      return lex_literal(source, u8iterator(u8"alse"sv));
    case 'n':
      return lex_literal(source, u8iterator(u8"ull"sv));
    case 't':
      return lex_literal(source, u8iterator(u8"rue"sv));
    case '"':
      return lex_string(source);
    case '[':
      return lex_array(source);
    case '{':
      return lex_object(source);
    }
    return JSON_ERR_lex_value;
  }
};
