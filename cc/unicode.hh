#pragma once
#include <ranges>

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
  constexpr u8iterator(const R &code_units) noexcept
      : curr(std::ranges::begin(code_units)), end(std::ranges::end(code_units)), codepoint(next()) {
  }
  constexpr int operator*() const noexcept { return codepoint; }
  constexpr u8iterator &operator++() noexcept {
    codepoint = next();
    return *this;
  }
  constexpr u8iterator operator++(int) noexcept {
    auto old = *this;
    ++*this;
    return old;
  }
};
