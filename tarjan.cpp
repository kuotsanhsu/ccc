// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;

class vertex {
  using stack_frame = vertex *;
  const stack_frame *low_link;
  stack_frame *prev;
  const struct children {
    const children *next;
    struct vertex *vertex;
  } *children;

public:
  [[nodiscard]] std::ptrdiff_t index() const noexcept { return this - vertex_begin; }
  stack_frame *stack(stack_frame *prev, stack_frame *&top) {
    this->prev = prev;
    low_link = top;
    *top = this;
    return top++;
  }
  [[nodiscard]] bool visited() const noexcept { return low_link != nullptr; }
  bool same_low_link(const stack_frame *curr) { return low_link == curr; }
  void update_low_link(const vertex *other) { low_link = std::min(low_link, other->low_link); }
  void bump_low_link(const stack_frame *high) {
    low_link = high;
#ifndef NDEBUG
    std::clog << ' ' << index();
#endif
  }
  void add_child(vertex &child) {
    static std::array<struct children, edge_count_max> edge_pool;
    static auto *edge_pool_ptr = edge_pool.begin();

    *edge_pool_ptr = {.next = children, .vertex = &child};
    children = edge_pool_ptr++;
  }
  vertex *consume_child() {
    vertex *vertex = nullptr;
    if (auto &child = children) {
      vertex = child->vertex;
      child = child->next;
    }
    return vertex;
  }
  static stack_frame *go_back(stack_frame *&curr) { return curr = (*curr)->prev; }

  friend auto &operator<<(std::ostream &cos, const vertex &vertex) {
    cos << '(';
    if (vertex.prev != nullptr) {
      cos << vertex.prev - low_link_begin;
    } else {
      cos << '*';
    }
    cos << " <- " << vertex.low_link - low_link_begin << ") " << vertex.index() << ':';
    for (const auto *children = vertex.children; children != nullptr; children = children->next) {
      cos << ' ' << children->vertex->index();
    }
    return cos;
  }

  static std::span<vertex> get_vertices(std::size_t count) {
    assert(count <= vertices.size());
    return {vertices.begin(), count};
  }
  static std::span<stack_frame> get_stack(std::size_t count) {
    assert(count <= dfs_stack.size());
    return {dfs_stack.begin(), count};
  }

private:
  static constinit std::array<vertex, vertex_count_max> vertices;
  static constinit const vertex *const vertex_begin;
  static constinit std::array<stack_frame, vertex_count_max> dfs_stack;
  static constinit const stack_frame *const low_link_begin;
};

constinit std::array<vertex, vertex_count_max> vertex::vertices{};
constinit const vertex *const vertex::vertex_begin = vertices.cbegin();
constinit std::array<vertex::stack_frame, vertex_count_max> vertex::dfs_stack{};
constinit const vertex::stack_frame *const vertex::low_link_begin = dfs_stack.cbegin();

namespace {
void preallocated(std::size_t vertex_count, std::ranges::input_range auto &&edges) {
  auto vertices = vertex::get_vertices(vertex_count);
  for (const auto [source, target] : edges) {
    vertices[source].add_child(vertices[target]);
  }

  std::vector<std::ptrdiff_t> reverse_component_sizes;
  auto dfs_stack = vertex::get_stack(vertex_count);
  auto *sorted = dfs_stack.end().base();
  for (auto *top = dfs_stack.begin().base(); auto &vertex : vertices) {
    // assert(top == dfs_stack.begin());
    // __builtin_debugtrap();
    if (vertex.visited()) {
      continue;
    }
    for (auto *curr = vertex.stack(nullptr, top);;) {
#ifndef NDEBUG
      std::clog << **curr << '\n';
#endif
      while (auto *vertex = (*curr)->consume_child()) {
        if (vertex->visited()) {
          (*curr)->update_low_link(vertex);
        } else {
          curr = vertex->stack(curr, top);
          goto end;
        }
      }
      if ((*curr)->same_low_link(curr)) {
        reverse_component_sizes.push_back(top - curr);
#ifndef NDEBUG
        std::clog << '[' << reverse_component_sizes.back() << ']';
#endif
        while (top != curr) {
          (*--sorted = *--top)->bump_low_link(sorted);
        }
#ifndef NDEBUG
        std::clog << '\n';
#endif
        if (!vertex::go_back(curr)) {
          break;
        }
      } else {
        auto *vertex = *curr;
        (*vertex::go_back(curr))->update_low_link(vertex);
      }
    end:
    }
  }
  // assert(sorted == dfs_stack.begin());

  std::cout << reverse_component_sizes.size() << '\n';
  for (auto component_size : std::views::reverse(reverse_component_sizes)) {
    std::cout << component_size;
    while (component_size--) {
      std::cout << ' ' << (*sorted++)->index();
    }
    std::cout << '\n';
  }
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
