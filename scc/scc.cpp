#include <array>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <stack>
#include <tuple>
#include <vector>

constexpr std::vector<std::vector<int>> scc(const int N,
                                            const std::span<const std::pair<int, int>> edges) {
  // const auto M{edges.size()};
  class Vertex {
    int index{-1};
    int lowlink{-1};

  public:
    bool onStack{false};
    std::vector<Vertex *> successors{};

    void visit(const int index) noexcept {
      lowlink = this->index = index;
      onStack = true;
    }

    void min_lowlink(const Vertex &other) { lowlink = std::min(lowlink, other.lowlink); }

    [[nodiscard]] bool visited() const noexcept { return index != -1; }
    [[nodiscard]] bool root() const noexcept { return index == lowlink; }
  };
  std::vector<Vertex> vertices(N);
  for (const auto [from, to] : edges) {
    vertices[from].successors.push_back(&vertices[to]);
  }

  std::stack<Vertex *> S;
  std::vector<std::vector<int>> components;
  const auto strongconnect = [front = &vertices.front(), &vertices, &S,
                              &components](this auto &&self, Vertex *const v) -> void {
    v->visit(S.size());
    S.push(v);
    for (auto w : v->successors) {
      if (!w->visited()) {
        self(w);
        v->min_lowlink(*w);
      } else if (w->onStack) {
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
        w->onStack = false;
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
  std::array<std::pair<int, int>, 500'000> edges;
  std::istream_iterator<int> ints(std::cin);
  const auto N = *ints++;
  const auto M = *ints++;
  auto last = edges.begin();
  for (const auto _ : std::views::iota(0, M)) {
    const auto a = *ints++;
    const auto b = *ints++;
    *last++ = {a, b};
  }
  std::cout << std::ranges::reverse_view(scc(N, {edges.begin(), last}));
}
