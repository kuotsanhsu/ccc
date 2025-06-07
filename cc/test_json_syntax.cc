#include <cassert>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
class u8iterator {
  std::ranges::iterator_t<R> curr;
  std::ranges::sentinel_t<R> end;
  int codepoint;
  constexpr int next() noexcept;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = int;
  constexpr u8iterator(const R &r) noexcept;
  constexpr int operator*() const;
  constexpr u8iterator &operator++();
  constexpr u8iterator operator++(int);
};

static_assert(std::input_iterator<u8iterator<std::u8string>>);
static_assert(std::input_iterator<u8iterator<std::u8string_view>>);

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
constexpr u8iterator<R>::u8iterator(const R &r) noexcept : curr(std::begin(r)), end(std::end(r)) {
  codepoint = next();
}

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
constexpr int u8iterator<R>::operator*() const {
  if (codepoint < 0) {
    throw codepoint;
  }
  return codepoint;
}

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
constexpr u8iterator<R> &u8iterator<R>::operator++() {
  if (codepoint < 0) {
    throw codepoint;
  }
  codepoint = next();
  return *this;
}

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
constexpr u8iterator<R> u8iterator<R>::operator++(int) {
  auto old = *this;
  ++*this;
  return old;
}

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, char8_t>
constexpr int u8iterator<R>::next() noexcept {
  if (curr == end)
    return -2;
  const int a = *curr++;
  if (a < 0x80) {
    // 00..7F
    // 0xxx'xxxx
    return a;
  }
  if (a < 0xC2 || 0xF4 < a)
    return -3;
  if (curr == end)
    return -2;
  const int b = *curr ^ 0x80;
  if (b >> 6)
    return -3;
  ++curr;
  int code_point = a << 6 ^ b;
  if (a < 0xE0) {
    //	C2..DF		80..BF
    //	110x'xxxx	10xx'xxxx
    return code_point ^ (0xC0 << 6);
  }

  switch (a) {
  case 0xE0: //	E0	A0..BF	80..BF
    if (b < 0x20)
      return -3;
    break;
  case 0xED: //	ED	80..9F	80..BF
    if (b >= 0x20)
      return -3;
    break;
  case 0xF0: //	F0	90..BF	80..BF	80..BF
    if (b < 0x10)
      return -3;
    break;
  case 0xF4: //	F4	80..8F	80..BF	80..BF
    if (b >= 0x10)
      return -3;
    break;
  }

  if (curr == end)
    return -2;
  const int c = *curr ^ 0x80;
  if (c >> 6)
    return -3;
  ++curr;
  code_point = (code_point << 6) ^ c;
  if (a < 0xF0) {
    //	E0		A0..BF		80..BF
    //	E1..EC		80..BF		80..BF
    //	ED		80..9F		80..BF
    //	EE..EF		80..BF		80..BF
    //	1110'xxxx	10xx'xxxx	10xx'xxxx
    return code_point ^ (0xE0 << 12);
  }

  if (curr == end)
    return -2;
  const int d = *curr ^ 0x80;
  if (d >> 6)
    return -3;
  ++curr;
  code_point = (code_point << 6) ^ d;
  //	F0		90..BF		80..BF		80..BF
  //	F1..F3		80..BF		80..BF		80..BF
  //	F4		80..8F		80..BF		80..BF
  //	1111'0xxx	10xx'xxxx	10xx'xxxx	10xx'xxxx
  return code_point ^ (0xF0 << 18);
}

template <typename R> consteval u8iterator<R> next(u8iterator<R> it) { return ++it; }

int main() {
  constexpr std::u8string_view hello = u8"hello world";
  constexpr u8iterator h(hello);
  static_assert(*h == 'h');
  constexpr auto e = next(h);
  static_assert(*e == 'e');
  constexpr auto l = next(e);
  static_assert(*l == 'l');
  constexpr auto o = next(next(l));
  using namespace std::string_view_literals;
  for (auto it = o; const auto c : u8"o world"sv) {
    assert(*it++ == c);
  }
}
