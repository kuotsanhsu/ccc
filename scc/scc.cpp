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
  static inline std::array<Vertex *, max_size> S;
  Vertex **stack = S.begin();
  std::vector<std::vector<Vertex *>> components;

  constexpr void tarjan(Vertex *const v) {
    if (v->rindex) {
      return;
    }
    *stack++ = v;
    const auto rindex = v->rindex = stack;
    for (auto w : v->successors) {
      tarjan(w);
      v->rindex = std::min(v->rindex, w->rindex);
    }
    if (v->rindex == rindex) {
      std::vector<Vertex *> component;
      while (true) {
        const auto w = *--stack;
        component.push_back(w);
        w->rindex = S.end();
        if (v == w) {
          break;
        }
      }
      components.push_back(component);
    }
  }

public:
  constexpr scc(const std::span<Vertex> vertices) noexcept {
    for (auto &vertex : vertices) {
      tarjan(&vertex);
    }
  }

  [[nodiscard]] constexpr auto result() const { return std::ranges::reverse_view(components); }
};

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  static std::array<Vertex, max_size> vertices;
  std::istream_iterator<size_t> ints(std::cin);
  const auto N = *ints++;
  const auto M = *ints++;
  for (const auto _ : std::views::iota(size_t{}, M)) {
    auto &from = vertices[*ints++];
    auto &to = vertices[*ints++];
    from.successors.push_back(&to);
  }

  const auto front = std::begin(vertices);
  scc tarjan({front, N});
  const auto components = tarjan.result();

  std::cout << std::ranges::size(components) << '\n';
  for (const auto &component : components) {
    std::cout << std::ranges::size(component);
    for (const auto v : component) {
      std::cout << ' ' << v - front;
    }
    std::cout << '\n';
  }
}
