#include <array>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <stack>
#include <vector>

constexpr size_t max_size{500'000};

struct Vertex {
  size_t rindex{0};
  std::vector<Vertex *> successors{};
};

class scc {
  const Vertex *front;
  std::stack<Vertex *> S;
  std::vector<std::vector<int>> components;

  constexpr void tarjan(Vertex *const v) {
    if (v->rindex) {
      return;
    }
    S.push(v);
    const auto rindex = v->rindex = S.size();
    for (auto w : v->successors) {
      tarjan(w);
      v->rindex = std::min(v->rindex, w->rindex);
    }
    if (v->rindex == rindex) {
      std::vector<int> component;
      while (true) {
        auto w = S.top();
        S.pop();
        component.push_back(w - front);
        w->rindex = max_size;
        if (v == w) {
          break;
        }
      }
      components.push_back(component);
    }
  }

public:
  constexpr scc(const std::span<Vertex> vertices) noexcept : front(&vertices.front()) {
    for (auto &vertex : vertices) {
      tarjan(&vertex);
    }
  }

  [[nodiscard]] constexpr auto result() const { return std::ranges::reverse_view(components); }
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

  scc tarjan({std::begin(vertices), N});
  const auto components = tarjan.result();

  std::cout << std::ranges::size(components) << '\n';
  for (const auto &component : components) {
    std::cout << std::ranges::size(component);
    for (const auto v : component) {
      std::cout << ' ' << v;
    }
    std::cout << '\n';
  }
}
