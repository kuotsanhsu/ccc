#include <array>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <memory_resource>
#include <ranges>
#include <span>

constexpr size_t max_size{500'000};
std::array<std::byte, sizeof(nullptr) * 2 * max_size> buffer;
std::pmr::monotonic_buffer_resource mbr(buffer.data(), buffer.size());

struct vertex {
  vertex **rindex{nullptr};
  std::pmr::forward_list<vertex *> successors{&mbr};
  decltype(successors)::const_iterator last_successor{successors.cbefore_begin()};
};

class scc {
  static inline std::array<vertex *, max_size> stack;
  const vertex *front;
  vertex **top = stack.begin();
  vertex **component = stack.end();
  size_t K{0};

  constexpr void tarjan(vertex *const v) {
    if (v->rindex) {
      return;
    }
    const auto rindex = v->rindex = top;
    *top++ = v;
    for (auto w : v->successors) {
      tarjan(w);
      v->rindex = std::min(v->rindex, w->rindex);
    }
    if (v->rindex == rindex) {
      const auto last = component;
      do {
        (*--component = *--top)->rindex = last;
      } while (top != rindex);
      ++K;
    }
  }

public:
  constexpr scc(const std::span<vertex> vertices) noexcept : front(&vertices.front()) {
    for (auto &vertex : vertices) {
      tarjan(&vertex);
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const scc &scc) {
    os << scc.K << '\n';
    for (auto v = scc.component; v != scc.stack.end();) {
      const auto last = (*v)->rindex;
      os << last - v;
      for (auto w = last; w != v;) {
        os << ' ' << *--w - scc.front;
      }
      os << '\n';
      v = last;
    }
    return os;
  }
};

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  static std::array<vertex, max_size> vertices;
  std::istream_iterator<size_t> ints(std::cin);
  const auto N = *ints++;
  const auto M = *ints++;
  for (const auto _ : std::views::iota(size_t{}, M)) {
    auto &from = vertices[*ints++];
    auto &to = vertices[*ints++];
    from.last_successor = from.successors.insert_after(from.last_successor, &to);
  }
  std::cout << scc({std::begin(vertices), N});
}
