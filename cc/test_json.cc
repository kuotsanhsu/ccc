#include "json.hh"
#include "unicode.hh"
#include <algorithm>
#include <iostream>

template <utf8_code_unit_sequence R> constexpr bool test(R &&source) {
  json_visitor visitor;
  return json_parser(std::forward<R>(source) | to_codepoint, &visitor).lex_json_text() == -1;
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
  json_visitor visitor;
  json_parser parser(std::forward<R>(source) | to_codepoint, &visitor);
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

struct diagnostic_json_visitor : json_visitor {
  constexpr void end_json_text() final { std::cout << std::endl; }
  constexpr void end_false() final { std::cout.put('f'); }
  constexpr void end_null() final { std::cout.put('n'); }
  constexpr void end_true() final { std::cout.put('t'); }
  constexpr void begin_string() final { std::cout.put('<'); }
  constexpr void put_codepoint(int c) final {
    std::wcout.put(c); // FIXME: is mixing cout and wcout bad?
  }
  constexpr void end_string() final { std::cout.put('>'); }
  constexpr void begin_array() final { std::cout.put('['); }
  constexpr void end_array() final { std::cout.put(']'); }
  constexpr void begin_object() final { std::cout.put('{'); }
  constexpr void end_object() final { std::cout.put('}'); }
};

int main() {
  diagnostic_json_visitor visitor;
  assert(json_parser(std::views::all(file1) | to_codepoint, &visitor).lex_json_text() == -1);
  assert(json_parser(std::views::all(file2) | to_codepoint, &visitor).lex_json_text() == -1);
}
