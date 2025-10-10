#include <algorithm>
#include <cstddef>
#include <random>
#include <ranges>

int main() {
  constexpr size_t row_count = 9;
  constexpr size_t column_count = 9;
  constexpr size_t mine_count = 10;

  std::array<size_t, mine_count> mines{};
  {
    // static_assert(mine_count < mines.size());
    // std::ranges::fill_n(mines.begin(), mine_count, true);
  }
  constexpr auto indices = std::views::iota(size_t{}, row_count * column_count);
  std::mt19937_64 mt19937_64{std::random_device{}()};
  while (true) {
    // std::ranges::shuffle(mines, mt19937_64);
    std::ranges::sample(indices, mines.begin(), mines.size(), mt19937_64);
  }
}
// ░░▓▓
