#include "ansi.hpp"
#include "chess.hpp"
#include <csignal>
#include <iostream>
#include <termios.h>
#include <unistd.h>

class colored_piece {
  const char *glyph;
  ansi::fg fg;

  static constexpr std::array fgcolors{
      ansi::fg::bright(ansi::color::green),
      ansi::fg::bright(ansi::color::white),
  };

  // [Full-width characters](https://stackoverflow.com/a/8327034)
  static constexpr const char *full_width_latin(const piece piece, const bool is_white) noexcept {
    switch (piece) {
    case piece::empty:
      return "　";
    case piece::pawn:
      return is_white ? "Ｐ" : "ｐ";
    case piece::rook:
      return is_white ? "Ｒ" : "ｒ";
    case piece::knight:
      return is_white ? "Ｎ" : "ｎ";
    case piece::bishop:
      return is_white ? "Ｂ" : "ｂ";
    case piece::queen:
      return is_white ? "Ｑ" : "ｑ";
    case piece::king:
      return is_white ? "Ｋ" : "ｋ";
    }
  }

public:
  constexpr colored_piece(const piece piece, const bool is_white) noexcept
      : glyph(full_width_latin(piece, is_white)), fg(fgcolors[is_white]) {}

  friend std::ostream &operator<<(std::ostream &os, const colored_piece &cp) {
    return os << cp.fg << cp.glyph;
  }
};

// https://askubuntu.com/a/558422
std::ostream &operator<<(std::ostream &os, const configuration &config) {
  static_assert(std::to_underlying(piece::empty) == 0);
  std::array<piece, 64> board{};
  static_assert(~square{0} == 63);
  static_assert(~square{1} == 0b111'110);
  static_assert(~square{0b100'110} == 0b011'001);
  static_assert(~square{63} == 0);
  for (const auto [piece, shift] : config.get_white()) {
    board[~shift] = piece;
  }
  for (const auto [piece, shift] : config.get_black()) {
    board[~shift] = piece;
  }
  constexpr auto file_hint{"　ａｂｃｄｅｆｇｈ　"};
  constexpr auto bgcolors = std::views::repeat(std::array{
                                ansi::bg::bright(ansi::color::blue),
                                ansi::bg(ansi::color::blue),
                            }) |
                            std::views::join;
  auto bgcolor = std::ranges::begin(bgcolors);
  constexpr auto hint_color = ansi::sgr(ansi::style::reset, ansi::fg::bright(ansi::color::black));
  os << hint_color << file_hint << "\r\n";
  auto pos = uint64_t{1} << ~square{0};
  auto square = board.cbegin();
  for (const auto rank : {"８", "７", "６", "５", "４", "３", "２", "１"}) {
    os << rank;
    for (const auto _ : std::views::iota(0, 8)) {
      os << *bgcolor++ << colored_piece(*square++, pos & config.get_white().get_occupancy());
      pos >>= 1;
    }
    os << hint_color << rank << "\r\n";
    ++bgcolor;
  }
  return os << file_hint << ansi::style::reset;
}

// Note that tcsetattr() returns success if any of the requested changes could be successfully
// carried out. Therefore, when making multiple changes it may be necessary to follow this call with
// a further call to tcgetattr() to check that all changes have been performed successfully.
void assert_tcsetattr(const int fd, const int optional_actions, const termios &expected) {
  assert(tcsetattr(fd, optional_actions, &expected) == 0);
  termios actual{};
  assert(tcgetattr(fd, &actual) == 0);
  assert(actual.c_iflag == expected.c_iflag);
  assert(actual.c_oflag == expected.c_oflag);
  assert(actual.c_cflag == expected.c_cflag);
  assert(actual.c_lflag == expected.c_lflag);
}

void noecho() {
  termios t{};
  assert(tcgetattr(STDIN_FILENO, &t) == 0);
  static const auto initial_termios = t;
  std::atexit([] {
    std::cout << ansi::cursor_position(11, 1) << std::flush;
    assert_tcsetattr(STDIN_FILENO, TCSAFLUSH, initial_termios);
  });
  cfmakeraw(&t);
  t.c_lflag |= ISIG;
  assert_tcsetattr(STDIN_FILENO, TCSAFLUSH, t);

  struct sigaction act{
      .sa_handler = [](int) { exit(1); },
      .sa_flags = 0,
  };
  assert(sigemptyset(&act.sa_mask) == 0);
  for (struct sigaction oldact{}; const int signum : {SIGINT, SIGHUP, SIGTERM}) {
    assert(sigaction(signum, nullptr, &oldact) == 0);
    if (oldact.sa_handler != SIG_IGN) {
      assert(sigaction(signum, &act, nullptr) == 0);
    }
  }
}

struct coordinates {
  int file, rank;
};

std::ostream &operator<<(std::ostream &os, const coordinates &coord) {
  return os << ansi::cursor_position(10 - coord.rank, coord.file * 2 + 1);
}

void loop() {
  coordinates coord(0, 0);
  for (char ch = '\0'; read(STDIN_FILENO, &ch, 1) == 1;) {
    if ('a' <= ch && ch <= 'h') {
      if (coord.rank == 0) {
        coord.file = ch - 'a' + 1;
        std::cout << ansi::cursor_show << coord << std::flush;
      }
    } else if ('1' <= ch && ch <= '8') {
      if (coord.file != 0) {
        coord.rank = ch - '1' + 1;
        std::cout << coord << std::flush;
      }
    } else {
      switch (ch) {
      case '\n':
        break;
      case '\033':
        coord.file = coord.rank = 0;
        std::cout << ansi::cursor_hide << std::flush;
        break;
      }
    }
  }
}

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  noecho();

  constexpr configuration config;
  std::cout << ansi::hard_clear_screen << ansi::cursor_hide << config << std::flush;
  loop();
}
