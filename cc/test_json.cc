#include "unicode.hh"
#include <cassert>
#include <string_view>

enum class json_marker : char8_t {
  JSON_begin_array = '[',
  JSON_begin_object = '{',
  JSON_end_array = '\1',
  JSON_end_object = '\1',
  // JSON_name_separator = ':',
  JSON_value_separator = '\0',
  JSON_false = 'f',
  JSON_null = 'n',
  JSON_true = 't',
  JSON_e = '/', // -./0123456789
  JSON_begin_string = '"',
  JSON_end_string = u8'\xFF',
};

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

public:
  json_parser(R source) : source_iter(codepoint_view<R>(source).begin()) {}

  int lex_json_text() {
    int c = *source_iter++;
    if (c == 0xFEFF) { // Skip BOM.
      c = *source_iter++;
    }
    return lex_whitespace(lex_value(c));
  }

private:
  int lex_whitespace(int c) {
    for (;; c = *source_iter++) {
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

  template <utf8_code_unit_sequence T> int lex_literal(T &&literal) {
    int c = *source_iter++;
    for (const int d : literal | to_codepoint) {
      assert(d >= 0);
      if (c < 0) {
        break;
      }
      if (c != d) {
        return err_lex_literal;
      }
      c = *source_iter++;
    }
    return c;
  }

  int lex_4_xdigits() {
    for (int n = 4; n--;) {
      const int c = *source_iter++;
      if (c < 0) {
        return c;
      }
      if (isxdigit(c)) {
        continue;
      }
      return err_lex_xdigit;
    }
    return 0;
  }

  int lex_string() {
    while (1) {
      const int c = *source_iter++;
      if (c < 0)
        return c;
      switch (c) {
      case '"':
        return *source_iter++;
      case '\\': {
        const int c = *source_iter++;
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
          const int ret = lex_4_xdigits();
          if (ret < 0)
            return ret;
          break;
        }
        default:
          return err_lex_escape;
        }
        break;
      }
      default:
        // Control characters (U+0000 through U+001F) MUST be
        // escaped.
        if (c < 0x20)
          return err_lex_string;
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
      return err_lex_digit;
    do {
      c = *source_iter++;
    } while (isdigit(c));
    return c;
  }

  int lex_number_frac_exp(int c) {
    if (c == '.')
      c = lex_1_or_more_digits(*source_iter++);
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

  int lex_number_after_first_digit() {
    int c = *source_iter++;
    while (isdigit(c)) {
      c = *source_iter++;
    }
    return lex_number_frac_exp(c);
  }

  int lex_number_int_frac_exp() {
    const int c = *source_iter++;
    if (c < 0)
      return c;
    if ('1' <= c && c <= '9')
      return lex_number_after_first_digit();
    if (c == '0')
      return lex_number_frac_exp(*source_iter++);
    return err_lex_escape;
  }

  int lex_array() {
    for (int c = *source_iter++;;) {
      c = lex_value(c);
      if (c < 0)
        return c;
      c = lex_whitespace(c);
      if (c < 0)
        return c;
      switch (c) {
      case ']':
        return *source_iter++;
      case ',':
        c = *source_iter++;
        break;
      default:
        return err_lex_end_array_or_value_separator;
      }
    }
  }

  int lex_object() {
    for (int c = *source_iter++;;) {
      c = lex_whitespace(c);
      if (c < 0)
        return c;
      switch (c) {
      case '"':
        c = lex_string();
        break;
      default:
        return err_lex_object_member;
      }

      c = lex_whitespace(c);
      if (c < 0)
        return c;
      switch (c) {
      case ':':
        c = *source_iter++;
        break;
      default:
        return err_lex_object_name_separator;
      }

      c = lex_value(c);
      if (c < 0)
        return c;

      c = lex_whitespace(c);
      if (c < 0)
        return c;
      switch (c) {
      case '}':
        return *source_iter++;
      case ',':
        c = *source_iter++;
        break;
      default:
        return err_lex_end_object_or_value_separator;
      }
    }
  }

  int lex_value(int c) {
    using namespace std::string_view_literals;
    c = lex_whitespace(c);
    if (c < 0)
      return c;
    if ('1' <= c && c <= '9')
      return lex_number_after_first_digit();
    switch (c) {
    case '0':
      return lex_number_frac_exp(*source_iter++);
    case '-':
      return lex_number_int_frac_exp();
    case 'f':
      return lex_literal(u8"alse"sv);
    case 'n':
      return lex_literal(u8"ull"sv);
    case 't':
      return lex_literal(u8"rue"sv);
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

int main() {
  for (const std::u8string_view source : {
           u8"false",
           u8"null",
           u8"true",
           u8"\"Hello world!\"",
           u8"42",
       }) {
    assert(json_parser(source).lex_json_text() == -1);
  }
  constexpr char8_t file1[] = {
#embed "Image.json"
  };
  constexpr char8_t file2[] = {
#embed "San_Francisco_and_Sunnyvale.json"
  };
  static_assert(std::size(file1) == 330);
  static_assert(std::size(file2) == 485);
  assert(json_parser(std::views::all(file1)).lex_json_text() == -1);
  assert(json_parser(std::views::all(file2)).lex_json_text() == -1);
  constexpr std::string_view image = // clang-format off
      "{"
				"Image\xFF" "{"
					"Width\xFF" "800"
				"\0"
					"Height\xFF" "600"
				"\0"
					"Title\xFF" "\"View from 15th Floor\xFF"
				"\0"
					"Thumbnail\xFF" "{"
						"Url\xFF" "\"http://www.example.com/image/481989943\xFF"
					"\0"
						"Height\xFF" "125"
					"\0"
						"Width\xFF" "100"
					"\1"
				"\0"
					"Animated\xFF" "f"
				"\0"
					"IDs\xFF" "["
						"116"
					"\0"
						"943"
					"\0"
						"234"
					"\0"
						"38793"
					"\1"
				"\1"
			"\1"
	; // clang-format on
}
