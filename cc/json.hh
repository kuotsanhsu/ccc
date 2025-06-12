#include "unicode.hh"

struct json_visitor {
  constexpr virtual ~json_visitor() = default;
  constexpr virtual void begin_json_text() {}
  constexpr virtual void bom() {}
  constexpr virtual void end_json_text() {}
  constexpr virtual void begin_whitespace() {}
  constexpr virtual void end_whitespace() {}
  constexpr virtual void begin_false() {}
  constexpr virtual void end_false() {}
  constexpr virtual void begin_null() {}
  constexpr virtual void end_null() {}
  constexpr virtual void begin_true() {}
  constexpr virtual void end_true() {}
  constexpr virtual void begin_string() {}
  constexpr virtual void codepoint(int c) {}
  constexpr virtual void end_string() {}
  constexpr virtual void begin_array() {}
  constexpr virtual void end_array() {}
  constexpr virtual void begin_object() {}
  constexpr virtual void end_object() {}
  /** Zero can take one of these two forms: `0`, `-0`. */
  constexpr virtual void begin_int(bool minus) {}
  constexpr virtual void end_int() {}
  constexpr virtual void begin_frac() {}
  constexpr virtual void end_frac() {}
  /** The grammar for `exp` is very laxed; these are all valid: `+000`, `-00001`. */
  constexpr virtual void begin_exp(bool minus) {}
  constexpr virtual void end_exp() {}
  constexpr virtual void digit(char c) {}
};

/**
 * Guiding principles:
 * 1. A negative codepoint SHOULD NOT end any lexeme/token/subtree OTHER THAN numbers, whitespaces
 * or json-text.
 * 2. Numbers are ended by the first non-numeric codepoint including a negative codepoint.
 * 3. TODO: disable recursion and use custom allocated stacks instead.
 */
template <codepoint_sequence R> class json_parser {
  std::ranges::iterator_t<R> source_iter;
  json_visitor *visitor;

  enum {
    err_lex_value = -10,
    err_lex_literal = -11,
    err_lex_object_member = -12,
    err_lex_object_name_separator = -13,
    err_lex_end_object_or_value_separator = -14,
    err_lex_end_array_or_value_separator = -15,
    err_lex_xdigit = -16,
    err_lex_digit = -17,
    err_lex_escape = -20,
    err_lex_string = -21,
  };

  constexpr static bool isdigit(int c) noexcept { return '0' <= c && c <= '9'; }

  constexpr static bool isxdigit(int c) noexcept {
    return isdigit(c) || 'A' <= c && c <= 'F' || 'a' <= c && c <= 'f';
  }

public:
  constexpr json_parser(R source, json_visitor *visitor)
      : source_iter(std::ranges::begin(source)), visitor(visitor) {}

  /** Repeated calls to lex_json_text will always return -1. */
  constexpr int lex_json_text() {
    visitor->begin_json_text();
    int c = *source_iter++;
    if (constexpr int BOM = 0xFEFF; c == BOM) { // Skip BOM.
      visitor->bom();
      c = *source_iter++;
    }
    c = lex_whitespace(lex_value(c));
    visitor->end_json_text();
    return c;
  }

private:
  constexpr int lex_whitespace(int c) {
    for (visitor->begin_whitespace();; c = *source_iter++) {
      switch (c) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        break;
      default:
        visitor->end_whitespace();
        return c;
      }
    }
  }

  template <int K, int... Ls> constexpr int lex_literal() {
    begin_literal<K>();
    int c = *source_iter++;
    for (const int d : {Ls...}) {
      if (c < 0) {
        break;
      }
      if (c != d) {
        return err_lex_literal;
      }
      c = *source_iter++;
    }
    end_literal<K>();
    return c;
  }

  template <int K> constexpr void begin_literal();
  template <int K> constexpr void end_literal();
  template <> constexpr void begin_literal<'f'>() { visitor->begin_false(); }
  template <> constexpr void end_literal<'f'>() { visitor->end_false(); }
  template <> constexpr void begin_literal<'n'>() { visitor->begin_null(); }
  template <> constexpr void end_literal<'n'>() { visitor->end_null(); }
  template <> constexpr void begin_literal<'t'>() { visitor->begin_true(); }
  template <> constexpr void end_literal<'t'>() { visitor->end_true(); }

  constexpr int lex_4_xdigits() {
    for (int n = 4; n--;) {
      if (const int c = *source_iter++; c < 0) {
        return c;
      } else if (!isxdigit(c)) {
        return err_lex_xdigit;
      }
    }
    return 0; // FIXME: return UTF-16 code unit instead of 0.
  }

  constexpr int lex_string() {
    visitor->begin_string();
    while (true) {
      const int c = *source_iter++;
      if (c < 0) {
        return c;
      }
      switch (c) {
      case '"':
        visitor->end_string();
        return *source_iter++;
      case '\\': {
        const int c = *source_iter++;
        if (c < 0) {
          return c;
        }
        switch (c) {
        case '"':
        case '\\':
        case '/':
          visitor->codepoint(c);
          break;
        case 'b':
          visitor->codepoint('\b');
          break;
        case 'f':
          visitor->codepoint('\f');
          break;
        case 'n':
          visitor->codepoint('\n');
          break;
        case 'r':
          visitor->codepoint('\r');
          break;
        case 't':
          visitor->codepoint('\t');
          break;
        case 'u':
          // TODO: visitor->begin_utf16()?
          if (const int ret = lex_4_xdigits(); ret < 0) {
            return ret;
          }
          // TODO: decode UTF-16, possibly anticipating a low surrogate.
          break;
        default:
          return err_lex_escape;
        }
        break;
      }
      default:
        if (c < 0x20) { // Control characters (U+0000 through U+001F) MUST be escaped.
          return err_lex_string;
        }
        consteval_assert(c != 0x22);     // Quotation mark (U+22) WILL NOT appear here.
        consteval_assert(c != 0x5C);     // Reverse solidus (U+5C) WILL NOT appear here.
        consteval_assert(c < 0x11'0000); // WILL be a valid code point.
        visitor->codepoint(c);
      }
    }
  }

  template <void (json_visitor::*end_func)()> constexpr int lex_1_or_more_digits(char c) {
    if (c < 0) {
      return c;
    }
    if (!isdigit(c)) {
      return err_lex_digit;
    }
    do {
      visitor->digit(c);
      c = *source_iter++;
    } while (isdigit(c));
    (visitor->*end_func)();
    return c;
  }

  constexpr int lex_number_after_first_digit(char first_digit, bool minus) {
    consteval_assert(isdigit(first_digit));
    visitor->begin_int(minus);
    visitor->digit(first_digit);
    int c = *source_iter++;
    if (first_digit != '0') { // Here, the value 0 MUST be represented by a single digit.
      for (; isdigit(c); c = *source_iter++) {
        visitor->digit(c);
      }
    }
    if (c < 0) {
      return c;
    }
    visitor->end_int();

    if (c == '.') {
      visitor->begin_frac();
      c = lex_1_or_more_digits<&json_visitor::end_frac>(*source_iter++);
    }
    switch (c) {
    case 'E':
    case 'e': {
      bool minus = false;
      switch (c = *source_iter++; c) {
      case '-':
        minus = true;
        [[fallthrough]];
      case '+':
        c = *source_iter++;
      }
      visitor->begin_exp(minus);
      c = lex_1_or_more_digits<&json_visitor::end_exp>(c);
    }
    }
    return c;
  }

  constexpr int lex_int_frac_exp() {
    if (const int c = *source_iter++; c < 0) {
      return c;
    } else if (isdigit(c)) {
      return lex_number_after_first_digit(c, true);
    }
    return err_lex_digit;
  }

  constexpr int lex_array() {
    visitor->begin_array();
    for (int c = *source_iter++;;) {
      if (c = lex_value(c); c < 0) {
        return c;
      }
      if (c = lex_whitespace(c); c < 0) {
        return c;
      }
      switch (c) {
      case ']':
        visitor->end_array();
        return *source_iter++;
      case ',':
        c = *source_iter++;
        break;
      default:
        return err_lex_end_array_or_value_separator;
      }
    }
  }

  constexpr int lex_object() {
    visitor->begin_object();
    for (int c = *source_iter++;;) {
      if (c = lex_whitespace(c); c < 0) {
        return c;
      }
      switch (c) {
      case '"':
        c = lex_string();
        break;
      default:
        return err_lex_object_member;
      }

      if (c = lex_whitespace(c); c < 0) {
        return c;
      }
      switch (c) {
      case ':':
        c = *source_iter++;
        break;
      default:
        return err_lex_object_name_separator;
      }

      if (c = lex_value(c); c < 0) {
        return c;
      }

      if (c = lex_whitespace(c); c < 0) {
        return c;
      }
      switch (c) {
      case '}':
        visitor->end_object();
        return *source_iter++;
      case ',':
        c = *source_iter++;
        break;
      default:
        return err_lex_end_object_or_value_separator;
      }
    }
  }

  constexpr int lex_value(int c) {
    if (c = lex_whitespace(c); c < 0) {
      return c;
    }
    if (isdigit(c)) {
      return lex_number_after_first_digit(c, false);
    }
    switch (c) {
    case '-':
      return lex_int_frac_exp();
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
