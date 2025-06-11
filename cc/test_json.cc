#include "unicode.hh"
#include <cassert>
#include <string_view>

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

  enum : char8_t {
    marker_begin_array = '[',
    marker_begin_object = '{',
    marker_end_array = '\1',
    marker_end_object = '\1',
    // marker_name_separator = ':',
    marker_value_separator = '\0',
    marker_false = 'f',
    marker_null = 'n',
    marker_true = 't',
    marker_e = '/', // -./0123456789
    marker_begin_string = '"',
    marker_end_string = u8'\xFF',
  };

  constexpr static bool isdigit(int c) { return '0' <= c && c <= '9'; }

  constexpr static bool isxdigit(int c) {
    return isdigit(c) || 'a' <= c && c <= 'f' || 'A' <= c && c <= 'F';
  }

  constexpr static bool isnumber(int c) { return '-' <= c && c <= '9'; }

public:
  constexpr json_parser(R source) : source_iter(codepoint_view<R>(source).begin()) {}

  constexpr int lex_json_text() {
    int c = *source_iter++;
    if (c == 0xFEFF) { // Skip BOM.
      c = *source_iter++;
    }
    return lex_whitespace(lex_value(c));
  }

private:
  constexpr int lex_whitespace(int c) {
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

  template <utf8_code_unit_sequence T> constexpr int lex_literal(T &&literal) {
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
    while (1) {
      const int c = *source_iter++;
      if (c < 0) {
        return c;
      }
      switch (c) {
      case '"':
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
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
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
    using namespace std::string_view_literals;
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

template <utf8_code_unit_sequence R> constexpr bool test(R &&source) {
  return json_parser(source).lex_json_text() == -1;
}

using namespace std::string_view_literals;
static_assert(test(u8"false"sv));
static_assert(test(u8"null"sv));
static_assert(test(u8"true"sv));
static_assert(test(u8"\"Hello world!\""sv));
static_assert(test(u8"42"sv));

constexpr char8_t file1[] = {
#embed "Image.json"
};
constexpr char8_t file2[] = {
#embed "San_Francisco_and_Sunnyvale.json"
};
static_assert(std::size(file1) == 330);
static_assert(std::size(file2) == 485);
static_assert(json_parser(std::views::all(file1)).lex_json_text() == -1);
static_assert(json_parser(std::views::all(file2)).lex_json_text() == -1);

constexpr std::u8string_view image = // clang-format off
		u8"{"
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

int main() {}
