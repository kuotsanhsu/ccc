// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <cstddef>
#include <forward_list>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;

struct vertex;

class state {
  static state *top;

  const state *low_link;
  vertex *parent;

  state(vertex *parent) noexcept : low_link(this), parent(parent) {}

public:
  static state *push_after(vertex *parent) noexcept { return new (top++) state(parent); }

  [[nodiscard]] const state *get_low_link() const noexcept { return low_link; }
  void set_low_link(const state *link) noexcept { low_link = std::min(low_link, link); }

  /// Effectively returns `parent`.
  [[nodiscard]] vertex *destruct(vertex *vertex) noexcept { return std::exchange(parent, vertex); }
  /// `vertex` must be called after `destruct`.
  static std::ranges::sized_range auto pop_after(state *link) noexcept {
    return std::ranges::subrange(link, std::exchange(top, link)) |
           std::views::transform(&state::parent);
    // | std::views::reverse;
  }
};

class vertex {
  static state *const max_link;
  static vertex *const vertex_end;

  state *link;
  std::pmr::forward_list<std::reference_wrapper<vertex>> children;

public:
  [[nodiscard]] vertex *child() noexcept {
    while (!children.empty()) {
      vertex &child = children.front();
      children.pop_front();
      if (child.link == nullptr) {
        child.link = state::push_after(this);
        return &child;
      }
      link->set_low_link(child.link);
    }
    return nullptr;
  }

  [[nodiscard]] vertex *parent() noexcept {
    const auto parent = link->destruct(this);
    const auto low_link = link->get_low_link();
    if (link == low_link) {
      const auto strong_component = state::pop_after(link);
      for (auto vertex : strong_component) {
        vertex->link = max_link;
      }
    } else {
      parent->link->set_low_link(low_link);
    }
    return parent;
  }

  void add_child(vertex &child) noexcept { children.emplace_front(&child); }

  static void print(std::ostream &cos) {
    cos << component_count << '\n';
    while (component_count--) {
      auto count = *index_iter++;
      cos << count;
      while (count--) {
        cos << ' ' << *index_iter++;
      }
      cos << '\n';
    }
  }

  [[nodiscard]] vertex *next() noexcept {
    for (auto curr = this; curr != vertex_end; ++curr) {
      if (curr->link == nullptr) {
        curr->link = state::push_after(nullptr);
        return curr;
      }
    }
    return nullptr;
  }
};

namespace {
void fff(vertex *curr) noexcept {
  while (true) {
    if (const auto child = curr->child()) {
      curr = child;
      continue;
    }
    const auto parent = curr->parent();
  }
}
void find_component(vertex &curr) noexcept {
  if (const auto child = curr.child()) {
    __attribute__((musttail)) return find_component(*child);
  }
  if (const auto parent = curr.parent()) {
    __attribute__((musttail)) return find_component(*parent);
  }
  if (const auto next = curr.next()) {
    __attribute__((musttail)) return find_component(*next);
  }
}

} // namespace

static_assert(sizeof(state) == sizeof(std::ptrdiff_t) * 2);
static_assert(alignof(state) == alignof(std::ptrdiff_t));
std::array<std::ptrdiff_t, vertex_count_max << 1> indices{};
state *state::top = reinterpret_cast<state *>(indices.begin());
state *const vertex::max_link = reinterpret_cast<state *>(indices.end());
// std::ptrdiff_t *vertex::index_iter = indices.end();
// std::size_t vertex::component_count = 0;

namespace {
void preallocated(std::size_t vertex_count, std::ranges::input_range auto &&edges) {
  std::pmr::vector<vertex> vertices(vertex_count);
  for (const auto [source, target] : edges) {
    vertices[source].add_child(vertices[target]);
  }
  vertex::print(std::cout);
  const auto vvv = vertices;
}
} // namespace

namespace std {
template <typename A, typename B> auto &operator>>(std::istream &cis, std::pair<A, B> &pair) {
  return cis >> pair.first >> pair.second;
}
} // namespace std

int main() {
  std::cin.tie(nullptr);
  std::ios_base::sync_with_stdio(false);
  std::istream_iterator<std::pair<int, int>> pairs(std::cin);
  const auto [vertex_count, edge_count] = *pairs++;
  preallocated(vertex_count, std::views::counted(pairs, edge_count));
}
