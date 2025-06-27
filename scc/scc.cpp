#include <array>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <vector>

constexpr size_t max_size{500'000};

struct Vertex {
  size_t rindex{0};
  std::vector<Vertex *> successors{};
};

class scc {
  const Vertex *front;
  std::array<Vertex *, max_size> stack;
  Vertex **top = stack.begin();
  Vertex **component = stack.end();
  std::array<size_t, max_size> component_sizes;
  size_t *szs = component_sizes.end();

  constexpr void tarjan(Vertex *const v) {
    if (v->rindex) {
      return;
    }
    *top++ = v;
    const auto rindex = v->rindex = top - stack.begin();
    for (auto w : v->successors) {
      tarjan(w);
      v->rindex = std::min(v->rindex, w->rindex);
    }
    if (v->rindex == rindex) {
      const auto last = top;
      while (true) {
        auto w = *--top;
        w->rindex = max_size;
        if (v == w) {
          break;
        }
      }
      *--szs = last - top;
      for (auto v = top; v != last; ++v) {
        *--component = *v;
      }
    }
  }

public:
  constexpr scc(const std::span<Vertex> vertices) noexcept : front(&vertices.front()) {
    for (auto &vertex : vertices) {
      tarjan(&vertex);
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const scc &scc) {
    os << scc.component_sizes.end() - scc.szs << '\n';
    auto v = scc.component;
    for (auto sz = scc.szs; sz != scc.component_sizes.end(); ++sz) {
      os << *sz;
      for (auto n = *sz; n--; ++v) {
        os << ' ' << *v - scc.front;
      }
      os << '\n';
    }
    return os;
  }
};

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  static Vertex vertices[max_size];
  std::istream_iterator<size_t> ints(std::cin);
  const auto N = *ints++;
  const auto M = *ints++;
  for (const auto _ : std::views::iota(size_t{}, M)) {
    auto &from = vertices[*ints++];
    auto &to = vertices[*ints++];
    from.successors.push_back(&to);
  }
  static scc scc({std::begin(vertices), N});
  std::cout << scc;
}
