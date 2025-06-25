#include "ansi.hpp"
#include <cassert>
#include <sstream>

int main() {
  std::ostringstream os;

  os.str({});
  os.clear();
  os << ansi::style::reset;
  assert(os.str() == "\033[0m");

  os.str({});
  os.clear();
  os << ansi::style::bold;
  assert(os.str() == "\033[1m");

  os.str({});
  os.clear();
  os << ansi::style::underline;
  assert(os.str() == "\033[4m");

  os.str({});
  os.clear();
  os << ansi::fg(ansi::color::red);
  assert(os.str() == "\033[31m");

  os.str({});
  os.clear();
  os << ansi::fg::bright(ansi::color::cyan);
  assert(os.str() == "\033[96m");

  os.str({});
  os.clear();
  os << ansi::bg(ansi::color::green);
  assert(os.str() == "\033[42m");

  os.str({});
  os.clear();
  os << ansi::bg::bright(ansi::color::magenta);
  assert(os.str() == "\033[105m");

  os.str({});
  os.clear();
  os << ansi::sgr(ansi::fg::bright(ansi::color::black), ansi::style::underline,
                  ansi::bg(ansi::color::white), ansi::style::italic);
  assert(os.str() == "\033[90;4;47;3m");
}
