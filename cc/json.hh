#include "unicode.hh"
#include <cassert>

template <utf8_code_unit_sequence R> class json_parser {
  codepoint_view<R>::iterator source_iter;

  enum {
    err_lex_value = -10,
    err_lex_literal = -11,
    err_lex_object_member = -12,
    err_lex_object_name_separator = -13,
    err_lex_end_object_or_value_separator = -14,
    err_lex_end_array_or_value_separator = -15,
    err_lex_xdigit = -16,
    err_lex_digit = -17,
    err_lex_number = -18,
    err_lex_escape = -19,
    err_lex_string = -20,
  };

  constexpr static bool isdigit(int c) noexcept { return '0' <= c && c <= '9'; }

  constexpr static bool isxdigit(int c) noexcept {
    return isdigit(c) || 'a' <= c && c <= 'f' || 'A' <= c && c <= 'F';
  }

public:
  constexpr json_parser(R source) : source_iter(codepoint_view<R>(source).begin()) {}

  /** Repeated calls to lex_json_text will always return -1. */
  constexpr int lex_json_text() {
    int c = *source_iter++;
    if (c == 0xFEFF) { // Skip BOM.
      c = *source_iter++;
    }
    return end_json_text(lex_whitespace(lex_value(c)));
  }

private:
  constexpr virtual int end_json_text(int c) { return c; }
  constexpr virtual void begin_whitespace(int c) {}
  constexpr virtual int end_whitespace(int c) { return c; }
  constexpr virtual int end_false(int c) { return c; }
  constexpr virtual int end_null(int c) { return c; }
  constexpr virtual int end_true(int c) { return c; }
  constexpr virtual void begin_string() {}
  constexpr virtual void put_codepoint(int c) {}
  constexpr virtual int end_string(int c) { return c; }
  constexpr virtual void begin_array() {}
  constexpr virtual int end_array(int c) { return c; }
  constexpr virtual void begin_object() {}
  constexpr virtual int end_object(int c) { return c; }

  template <int K> constexpr int end_literal(int c);
  template <> constexpr int end_literal<'f'>(int c) { return end_false(c); }
  template <> constexpr int end_literal<'n'>(int c) { return end_null(c); }
  template <> constexpr int end_literal<'t'>(int c) { return end_true(c); }

  constexpr int lex_whitespace(int c) {
    for (begin_whitespace(c);; c = *source_iter++) {
      switch (c) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        break;
      default:
        return end_whitespace(c);
      }
    }
  }

  template <int K, int... Ls> constexpr int lex_literal() {
    int c = *source_iter++;
    for (const int d : {Ls...}) {
      assert(d >= 0);
      if (c < 0) {
        break;
      }
      if (c != d) {
        return err_lex_literal;
      }
      c = *source_iter++;
    }
    return end_literal<K>(c);
  }

  constexpr int lex_4_xdigits() {
    for (int n = 4; n--;) {
      if (const int c = *source_iter++; c < 0) {
        return c;
      } else if (!isxdigit(c)) {
        return err_lex_xdigit;
      }
    }
    return 0;
  }

  constexpr int lex_string() {
    begin_string();
    while (1) {
      const int c = *source_iter++;
      if (c < 0) {
        return c;
      }
      switch (c) {
      case '"':
        return end_string(*source_iter++);
      case '\\': {
        const int c = *source_iter++;
        if (c < 0) {
          return c;
        }
        switch (c) {
        case '"':
        case '\\':
        case '/':
          put_codepoint(c);
          break;
        case 'b':
          put_codepoint('\b');
          break;
        case 'f':
          put_codepoint('\f');
          break;
        case 'n':
          put_codepoint('\n');
          break;
        case 'r':
          put_codepoint('\r');
          break;
        case 't':
          put_codepoint('\t');
          break;
        case 'u':
          if (const int ret = lex_4_xdigits(); ret < 0) {
            return ret;
          }
          break;
        default:
          return err_lex_escape;
        }
        break;
      }
      default:
        // Control characters (U+0000 through U+001F) MUST be
        // escaped.
        if (c < 0x20) {
          return err_lex_string;
        }
        // Quotation mark (U+22) WILL NOT appear here.
        assert(c != 0x22);
        // Reverse solidus (U+5C) WILL NOT appear here.
        assert(c != 0x5C);
        // WILL be a valid code point.
        assert(c < 0x110000);
        put_codepoint(c);
      }
    }
  }

  constexpr int lex_1_or_more_digits(int c) {
    if (c < 0) {
      return c;
    }
    if (!isdigit(c)) {
      return err_lex_digit;
    }
    do {
      c = *source_iter++;
    } while (isdigit(c));
    return c;
  }

  constexpr int lex_number_frac_exp(int c) {
    if (c == '.') {
      c = lex_1_or_more_digits(*source_iter++);
    }
    switch (c) {
    case 'e':
    case 'E':
      c = *source_iter++;
      switch (c) {
      case '-':
      case '+':
        c = *source_iter++;
      }
      c = lex_1_or_more_digits(c);
    }
    return c;
  }

  constexpr int lex_number_after_first_digit() {
    int c = *source_iter++;
    while (isdigit(c)) {
      c = *source_iter++;
    }
    return lex_number_frac_exp(c);
  }

  constexpr int lex_number_int_frac_exp() {
    if (const int c = *source_iter++; c < 0) {
      return c;
    } else if ('1' <= c && c <= '9') {
      return lex_number_after_first_digit();
    } else if (c == '0') {
      return lex_number_frac_exp(*source_iter++);
    }
    return err_lex_escape;
  }

  constexpr int lex_array() {
    begin_array();
    for (int c = *source_iter++;;) {
      c = lex_value(c);
      if (c < 0) {
        return c;
      }
      c = lex_whitespace(c);
      if (c < 0) {
        return c;
      }
      switch (c) {
      case ']':
        return end_array(*source_iter++);
      case ',':
        c = *source_iter++;
        break;
      default:
        return err_lex_end_array_or_value_separator;
      }
    }
  }

  constexpr int lex_object() {
    begin_object();
    for (int c = *source_iter++;;) {
      c = lex_whitespace(c);
      if (c < 0) {
        return c;
      }
      switch (c) {
      case '"':
        c = lex_string();
        break;
      default:
        return err_lex_object_member;
      }

      c = lex_whitespace(c);
      if (c < 0) {
        return c;
      }
      switch (c) {
      case ':':
        c = *source_iter++;
        break;
      default:
        return err_lex_object_name_separator;
      }

      c = lex_value(c);
      if (c < 0) {
        return c;
      }

      c = lex_whitespace(c);
      if (c < 0) {
        return c;
      }
      switch (c) {
      case '}':
        return end_object(*source_iter++);
      case ',':
        c = *source_iter++;
        break;
      default:
        return err_lex_end_object_or_value_separator;
      }
    }
  }

  constexpr int lex_value(int c) {
    c = lex_whitespace(c);
    if (c < 0) {
      return c;
    }
    if ('1' <= c && c <= '9') {
      return lex_number_after_first_digit();
    }
    switch (c) {
    case '0':
      return lex_number_frac_exp(*source_iter++);
    case '-':
      return lex_number_int_frac_exp();
    case 'f':
      return lex_literal<'f', 'a', 'l', 's', 'e'>();
    case 'n':
      return lex_literal<'n', 'u', 'l', 'l'>();
    case 't':
      return lex_literal<'t', 'r', 'u', 'e'>();
    case '"':
      return lex_string();
    case '[':
      return lex_array();
    case '{':
      return lex_object();
    }
    return err_lex_value;
  }
};
