#include <array>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <vector>

constexpr size_t max_size{500'000};

struct Vertex {
  Vertex **rindex{nullptr};
  std::vector<Vertex *> successors{};
};

class scc {
  const Vertex *front;
  std::array<Vertex *, max_size> stack;
  Vertex **top = stack.begin();
  Vertex **component = stack.end();
  size_t K{0};

  constexpr void tarjan(Vertex *const v) {
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
      for (auto pw = rindex; pw != top; ++pw) {
        *--component = *pw;
        (*pw)->rindex = last;
      }
      top = rindex;
      ++K;
    }
  }

public:
  constexpr scc(const std::span<Vertex> vertices) noexcept : front(&vertices.front()) {
    for (auto &vertex : vertices) {
      tarjan(&vertex);
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const scc &scc) {
    os << scc.K << '\n';
    for (auto v = scc.component; v != scc.stack.end();) {
      const auto last = (*v)->rindex;
      os << last - v;
      while (v != last) {
        os << ' ' << *v++ - scc.front;
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
