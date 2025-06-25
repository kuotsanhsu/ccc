#pragma once
#include <ostream>

namespace ansi {

// Rows and columns are 1-based.
class cursor_position {
  int row, col;

public:
  constexpr cursor_position(const int row, const int col) noexcept : row(row), col(col) {}

  friend std::ostream &operator<<(std::ostream &os, const cursor_position &position) {
    return os << "\033[" << position.row << ';' << position.col << 'H';
  }
};

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

enum class style {
  reset = 0,
  normal = reset,
  bold = 1,
  increased_intensity = 1,
  faint = 2,
  decreased_intensity = faint,
  italic = 3,
  underline = 4,
  slow_blink = 5,
  blink = slow_blink,
  rapid_blink = 6,
  invert = 7,
  conceal = 8,
  crossed_out = 9,
  strike = crossed_out,
  primary_font = 10,
  fraktur = 20,
  gothic = fraktur,
  doubly_underlined = 21,
  normal_intensity = 22,
  not_italic_nor_blackletter = 23,
  // Neither singly nor doubly underlined
  no_underlined = 24,
  not_blinking = 25,
  proportional_spacing = 26,
  not_reversed = 27,
  reveal = 28,
  not_concealed = reveal,
  not_crossed_out = 29,
};

enum class color : char { black = '0', red, green, yellow, blue, magenta, cyan, white };

class fg;
class bg;

template <typename T>
concept sgr_value_t = std::same_as<T, style> || std::same_as<T, fg> || std::same_as<T, bg>;

template <sgr_value_t... Ts> class sgr;

using namespace std::string_view_literals;

class fg {
  char sequence[17]{};

  template <sgr_value_t... Ts> friend class sgr;

public:
  static constexpr fg bright(const enum color color) noexcept { return {color, true}; }

  constexpr fg(const enum color color, const bool bright = false) noexcept {
    auto ch = std::begin(sequence);
    *ch++ = bright ? '9' : '3';
    *ch++ = std::to_underlying(color);
  }

  // 256 color
  constexpr fg(const uint8_t color) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy("38;5;"sv, ch).out;
    ch = std::to_chars(ch, ch + 3, color).ptr;
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
  }
};

class bg {
  char sequence[17]{};

  template <sgr_value_t... Ts> friend class sgr;

public:
  static constexpr bg bright(const enum color color) noexcept { return {color, true}; }

  constexpr bg(const enum color color, const bool bright = false) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy(bright ? "10"sv : "4"sv, ch).out;
    *ch++ = std::to_underlying(color);
  }

  // 256 color
  constexpr bg(const uint8_t color) noexcept {
    auto ch = std::begin(sequence);
    ch = std::ranges::copy("48;5;"sv, ch).out;
    ch = std::to_chars(ch, ch + 3, color).ptr;
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
  }
};

template <sgr_value_t... Ts> class sgr {
  std::tuple<Ts...> vs;

public:
  constexpr sgr(const Ts &...args) noexcept : vs(args...) {}

  friend std::ostream &operator<<(std::ostream &os, const sgr &sgr) {
    os << "\033";
    return [&os, &sgr]<auto... Is>(std::index_sequence<Is...>) constexpr -> std::ostream & {
      constexpr auto print = []<typename V>(const V &v) {
        if constexpr (std::same_as<V, style>) {
          return std::to_underlying(v);
        } else if constexpr (std::same_as<V, fg> || std::same_as<V, bg>) {
          return v.sequence;
        }
        std::unreachable();
      };
      return ((os << (Is ? ';' : '[') << print(std::get<Is>(sgr.vs))), ...);
    }(std::index_sequence_for<Ts...>{}) << 'm';
  }
};

template <sgr_value_t V> std::ostream &operator<<(std::ostream &os, const V &v) {
  return os << sgr(v);
}

} // namespace ansi
