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

  constexpr void visit(const int index) noexcept {
    this->index = -index;
    lowlink = index;
  }

  [[nodiscard]] constexpr bool visited() const noexcept { return index != 0; }
  [[nodiscard]] constexpr bool root() const noexcept { return -index == lowlink; }
  [[nodiscard]] constexpr bool onStack() const noexcept { return index < 0; }
  constexpr void offStack() noexcept { index = lowlink; }
  constexpr void min_lowlink(const Vertex &other) { lowlink = std::min(lowlink, other.lowlink); }
};

class tarjan {
  std::span<Vertex> vertices;
  std::stack<Vertex *> S;
  std::vector<std::vector<int>> components;

  constexpr void build_scc(Vertex *const v) {
    if (v->visited()) {
      return;
    }
    S.push(v);
    v->visit(S.size());
    for (auto w : v->successors) {
      build_scc(w);
      if (!w->onStack()) {
        // (v, w) is an edge pointing to an SCC already found and must be ignored.
        continue;
      }
      // Successor w is in stack S and hence in the current SCC.
      v->min_lowlink(*w);
    }
    if (v->root()) {
      std::vector<int> component;
      while (true) {
        const auto w = S.top();
        S.pop();
        w->offStack();
        component.push_back(w - &vertices.front());
        if (v == w) {
          break;
        }
      };
      components.push_back(component);
    }
  }

public:
  constexpr tarjan(const std::span<Vertex> vertices) noexcept : vertices(vertices) {
    for (auto &vertex : vertices) {
      build_scc(&vertex);
    }
  }

  [[nodiscard]] constexpr auto scc() const { return std::ranges::reverse_view(components); }
};

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

  tarjan tarjan({std::begin(vertices), N});
  const auto components = tarjan.scc();

  std::cout << std::ranges::size(components) << '\n';
  for (const auto &component : components) {
    std::cout << std::ranges::size(component);
    for (const auto v : component) {
      std::cout << ' ' << v;
    }
    std::cout << '\n';
  }
}
