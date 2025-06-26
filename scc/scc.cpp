#include <array>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <stack>
#include <vector>

class Vertex {
  int index{};
  int lowlink{};

public:
  std::vector<Vertex *> successors{};

  void visit(const int index) noexcept {
    this->index = -index;
    lowlink = index;
  }

  void min_lowlink(const Vertex &other) { lowlink = std::min(lowlink, other.lowlink); }

  [[nodiscard]] bool visited() const noexcept { return index != 0; }
  [[nodiscard]] bool root() const noexcept { return -index == lowlink; }
  [[nodiscard]] bool onStack() const noexcept { return index < 0; }
  void offStack() noexcept { index = lowlink; }
};

constexpr std::vector<std::vector<int>> scc(const std::span<Vertex> vertices) {
  std::stack<Vertex *> S;
  std::vector<std::vector<int>> components;
  const auto strongconnect = [front = &vertices.front(), vertices, &S,
                              &components](this auto &&self, Vertex *const v) -> void {
    S.push(v);
    v->visit(S.size());
    for (auto w : v->successors) {
      if (!w->visited()) {
        self(w);
        v->min_lowlink(*w);
      } else if (w->onStack()) {
        // Successor w is in stack S and hence in the current SCC. If w is not on stack, then (v, w)
        // is an edge pointing to an SCC already found and must be ignored.
        v->min_lowlink(*w);
      }
    }
    if (v->root()) {
      std::vector<int> component;
      while (true) {
        const auto w = S.top();
        S.pop();
        w->offStack();
        component.push_back(w - front);
        if (v == w) {
          break;
        }
      };
      components.push_back(component);
    }
  };
  for (auto &vertex : vertices) {
    if (!vertex.visited()) {
      strongconnect(&vertex);
    }
  }
  return components;
}

template <typename R>
concept nested_sized_ranges =
    std::ranges::sized_range<R> && std::ranges::sized_range<std::ranges::range_value_t<R>>;

std::ostream &operator<<(std::ostream &os, const nested_sized_ranges auto &&result) {
  std::cout << std::ranges::size(result) << '\n';
  for (const auto &component : result) {
    std::cout << std::ranges::size(component);
    for (const auto v : component) {
      std::cout << ' ' << v;
    }
    std::cout << '\n';
  }
  return os;
}

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  Vertex vertices[500'000];
  std::istream_iterator<size_t> ints(std::cin);
  const auto N = *ints++;
  const auto M = *ints++;
  for (const auto _ : std::views::iota(size_t{}, M)) {
    auto &from = vertices[*ints++];
    auto &to = vertices[*ints++];
    from.successors.push_back(&to);
  }
  std::cout << std::ranges::reverse_view(scc({std::begin(vertices), N}));
}
