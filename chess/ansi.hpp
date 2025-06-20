#pragma once
#include <algorithm>
#include <charconv>
#include <ostream>
#include <string>
#include <vector>

namespace ansi {

// Rows and columns are 1-based.
constexpr std::string cursor_position(const uint8_t row, const uint8_t col) {
  char array[] = "\033[row;colH";
  auto first = std::begin(array) + 2;
  const auto last = std::end(array);
  first = std::to_chars(first, last, row).ptr;
  *first++ = ';';
  first = std::to_chars(first, last, col).ptr;
  *first++ = 'H';
  *first = '\0';
  return array;
}

namespace impl {

template <char command> constexpr std::string move_cursor(const uint8_t offset) {
  char array[] = "\033[nnnX";
  auto first = std::begin(array) + 2;
  const auto last = std::end(array);
  first = std::to_chars(first, last, offset).ptr;
  *first++ = command;
  *first = '\0';
  return array;
}

} // namespace impl

constexpr auto cursor_up(const uint8_t offset) { return impl::move_cursor<'A'>(offset); }
constexpr auto cursor_down(const uint8_t offset) { return impl::move_cursor<'B'>(offset); }
constexpr auto cursor_forward(const uint8_t offset) { return impl::move_cursor<'C'>(offset); }
constexpr auto cursor_back(const uint8_t offset) { return impl::move_cursor<'D'>(offset); }
constexpr auto cursor_column(const uint8_t offset) { return impl::move_cursor<'G'>(offset); }
constexpr auto cursor_hide{"\033[?25l"};
constexpr auto cursor_show{"\033[?25h"};
constexpr auto cursor_steady_block{"\033[0 q"};
constexpr auto cursor_blinking_block{"\033[1 q"};
constexpr auto cursor_reset{"\033[H"};

constexpr auto clear_screen{"\033[2J"};
constexpr auto hard_clear_screen{"\033[3J\033c"};
constexpr auto clear_line{"\033[2K"};

constexpr auto reset{"\033[m"};
// The control sequence `CSI n m`, named Select Graphic Rendition (SGR), sets display attributes.
#define SGR(n) ("\033[" n "m")
constexpr auto bold = SGR("1");
constexpr auto increased_intensity = bold;
constexpr auto faint = SGR("2");
constexpr auto decreased_intensity = faint;
constexpr auto italic = SGR("3");
constexpr auto underline = SGR("4");
constexpr auto slow_blink = SGR("5");
constexpr auto blink = slow_blink;
constexpr auto rapid_blink = SGR("6");
constexpr auto invert = SGR("7");
constexpr auto conceal = SGR("8");
constexpr auto crossed_out = SGR("9");
constexpr auto strike = crossed_out;
constexpr auto primary_font = SGR("10");
constexpr auto fraktur = SGR("20");
constexpr auto gothic = fraktur;
constexpr auto doubly_underlined = SGR("21");
constexpr auto normal_intensity = SGR("22");
constexpr auto not_italic_nor_blackletter = SGR("23");
// Neither singly nor doubly underlined
constexpr auto no_underlined = SGR("24");
constexpr auto not_blinking = SGR("25");
constexpr auto proportional_spacing = SGR("26");
constexpr auto not_reversed = SGR("27");
constexpr auto reveal = SGR("28");
constexpr auto not_concealed = reveal;
constexpr auto not_crossed_out = SGR("29");

#undef SGR

enum class color : char { black = '0', red, green, yellow, blue, magenta, cyan, white };

using namespace std::string_literals;
using namespace std::string_view_literals;

class fg {
  char sequence[17];

public:
  constexpr operator auto() const noexcept { return sequence; }

  static constexpr fg bright(const enum color color) noexcept { return {color, true}; }

  constexpr fg(const enum color color, const bool bright = false) noexcept {
    auto ch = std::begin(sequence);
    *ch++ = bright ? '9' : '3';
    *ch++ = std::to_underlying(color);
    *ch++ = '\0';
  }

  // 256 color
  constexpr fg(const uint8_t color) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy("38;5;"sv, ch).out;
    ch = std::to_chars(ch, ch + 3, color).ptr;
    *ch = '\0';
  }

  // true color
  constexpr fg(const uint8_t red, const uint8_t green, const uint8_t blue) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy("38;2;"sv, ch).out;
    ch = std::to_chars(ch, ch + 3, red).ptr;
    *ch++ = ';';
    ch = std::to_chars(ch, ch + 3, green).ptr;
    *ch++ = ';';
    ch = std::to_chars(ch, ch + 3, blue).ptr;
    *ch = '\0';
  }
};

class bg {
  char sequence[17];

public:
  constexpr operator auto() const { return sequence; }

  static constexpr bg bright(const enum color color) noexcept { return {color, true}; }

  constexpr bg(const enum color color, const bool bright = false) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy(bright ? "10"sv : "4"sv, ch).out;
    *ch++ = std::to_underlying(color);
    *ch++ = '\0';
  }

  // 256 color
  constexpr bg(const uint8_t color) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy("48;5;"sv, ch).out;
    ch = std::to_chars(ch, ch + 3, color).ptr;
    *ch = '\0';
  }

  // true color
  constexpr bg(const uint8_t red, const uint8_t green, const uint8_t blue) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy("48;2;"sv, ch).out;
    ch = std::to_chars(ch, ch + 3, red).ptr;
    *ch++ = ';';
    ch = std::to_chars(ch, ch + 3, green).ptr;
    *ch++ = ';';
    ch = std::to_chars(ch, ch + 3, blue).ptr;
    *ch = '\0';
  }
};

template <typename T>
concept sgr_value_t = std::same_as<T, int> || std::same_as<T, fg> || std::same_as<T, bg>;

template <sgr_value_t... Ts> class sgr {
  std::tuple<Ts...> vs;

  template <size_t I>
    requires(I >= sizeof...(Ts))
  std::ostream &print(std::ostream &os) const {
    return os << 'm';
  }

  template <size_t I>
    requires(I < sizeof...(Ts))
  std::ostream &print(std::ostream &os) const {
    if constexpr (I != 0) {
      os << ';';
    }
    os << std::get<I>(vs);
    return print<I + 1>(os);
  }

public:
  constexpr sgr(const Ts &...args) noexcept : vs(args...) {}

  friend std::ostream &operator<<(std::ostream &os, const sgr &sgr) {
    os << "\033[";
    return sgr.print<0>(os);
  }
};

} // namespace ansi
