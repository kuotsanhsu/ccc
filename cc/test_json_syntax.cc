#include <iterator>
#include <string>
#include <string_view>

template <std::input_iterator I, std::sentinel_for<I> S = I>
  requires std::same_as<std::iter_value_t<I>, char8_t>
class u8iterator {
  I curr;
  I end;
  int codepoint;
  constexpr int next() noexcept;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = int;
  constexpr u8iterator(I begin, S end) noexcept;
  constexpr int operator*() const;
  constexpr u8iterator &operator++();
  constexpr void operator++(int) { ++*this; }
};

static_assert(std::input_iterator<u8iterator<std::u8string::iterator>>);
static_assert(std::input_iterator<u8iterator<std::u8string_view::const_iterator>>);
static_assert(std::input_iterator<u8iterator<char8_t *>>);
static_assert(std::input_iterator<u8iterator<const char8_t *>>);
static_assert(std::input_iterator<u8iterator<std::istream_iterator<char8_t>>>);

template <std::input_iterator I, std::sentinel_for<I> S>
  requires std::same_as<std::iter_value_t<I>, char8_t>
constexpr u8iterator<I, S>::u8iterator(I begin, S end) noexcept : curr(begin), end(end) {
  codepoint = next();
}

template <std::input_iterator I, std::sentinel_for<I> S>
  requires std::same_as<std::iter_value_t<I>, char8_t>
constexpr int u8iterator<I, S>::operator*() const {
  if (codepoint < 0) {
    throw codepoint;
  }
  return codepoint;
}

template <std::input_iterator I, std::sentinel_for<I> S>
  requires std::same_as<std::iter_value_t<I>, char8_t>
constexpr u8iterator<I, S> &u8iterator<I, S>::operator++() {
  if (codepoint < 0) {
    throw codepoint;
  }
  codepoint = next();
  return *this;
}

template <std::input_iterator I, std::sentinel_for<I> S>
  requires std::same_as<std::iter_value_t<I>, char8_t>
constexpr int u8iterator<I, S>::next() noexcept {
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

template <typename I, typename S> consteval u8iterator<I, S> next(u8iterator<I, S> it) {
  return ++it;
}

int main() {
  constexpr std::u8string_view hello = u8"hello";
  constexpr u8iterator h(hello.begin(), hello.end());
  static_assert(*h == 'h');
  constexpr auto e = next(h);
  static_assert(*e == 'e');
  constexpr auto l = next(e);
  static_assert(*l == 'l');
  constexpr auto o = next(next(l));
  static_assert(*o == 'o');
}
