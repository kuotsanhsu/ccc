#include "json.hh"
#include "unicode.hh"
#include <algorithm>
#include <iostream>

template <utf8_code_unit_sequence R> constexpr bool test(R &&source) {
  return json_parser(source | to_codepoint).lex_json_text() == -1;
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
static_assert(test(std::views::all(file1)));
static_assert(test(std::views::all(file2)));

template <utf8_code_unit_sequence R> constexpr bool repeated_parse(R &&source, int repetitions) {
  json_parser parser(source | to_codepoint);
  const auto rets = std::views::iota(0, repetitions) |
                    std::views::transform([&parser](int) { return parser.lex_json_text(); });
  return std::ranges::all_of(rets, [](int ret) { return ret == -1; });
}

static_assert(repeated_parse(std::views::all(file1), 3));

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

template <codepoint_sequence R> class diagnostic_json_parser : public json_parser<R> {
  constexpr virtual int end_json_text(int c) override {
    std::cout << std::endl;
    return c;
  }
  constexpr virtual int end_false(int c) override {
    std::cout.put('f');
    return c;
  }
  constexpr virtual int end_null(int c) override {
    std::cout.put('n');
    return c;
  }
  constexpr virtual int end_true(int c) override {
    std::cout.put('t');
    return c;
  }
  constexpr virtual void begin_string() override { std::cout.put('<'); }
  constexpr virtual void put_codepoint(int c) override { std::cout.put(c); }
  constexpr virtual int end_string(int c) override {
    std::cout.put('>');
    return c;
  }
  constexpr virtual void begin_array() override { std::cout.put('['); }
  constexpr virtual int end_array(int c) override {
    std::cout.put(']');
    return c;
  }
  constexpr virtual void begin_object() override { std::cout.put('{'); }
  constexpr virtual int end_object(int c) override {
    std::cout.put('}');
    return c;
  }

public:
  using json_parser<R>::json_parser;
};
template <typename R> diagnostic_json_parser(R) -> diagnostic_json_parser<R>;

int main() {
  assert(diagnostic_json_parser(std::views::all(file1) | to_codepoint).lex_json_text() == -1);
}
