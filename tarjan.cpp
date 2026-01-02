// https://judge.yosupo.jp/problem/scc
#include <cassert>
#include <iostream>
#include <ostream>
#include <ranges>
#include <vector>

// log2(500'000) is about 18.897
constexpr auto N = 500'000;
constexpr auto M = 500'000;

struct vertex;
struct children {
  const children *next;
  struct vertex *vertex;
};
using stack_frame = vertex *;
class vertex {
  const struct children *children;
  const stack_frame *low_link;
  stack_frame *prev;

public:
  [[nodiscard]] auto index() const noexcept { return this - vertex_begin; }
  stack_frame *stack(stack_frame *prev, stack_frame *&top) {
    this->prev = prev;
    low_link = top;
    *top = this;
    return top++;
  }
  [[nodiscard]] bool visited() const noexcept { return low_link; }
  bool same_low_link(const stack_frame *curr) { return low_link == curr; }
  void update_low_link(const vertex *other) { low_link = std::min(low_link, other->low_link); }
  void bump_low_link(const stack_frame *high) {
    low_link = high;
#ifndef NDEBUG
    std::clog << ' ' << index();
#endif
  }
  void add_child(vertex &child) {
    static std::array<struct children, M> edge_pool;
    static auto edge_pool_ptr = edge_pool.begin();

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

  friend auto &operator<<(std::ostream &os, const vertex &vertex) {
    os << '(';
    if (vertex.prev) {
      os << vertex.prev - low_link_begin;
    } else {
      os << '*';
    }
    os << " <- " << vertex.low_link - low_link_begin << ") " << vertex.index() << ':';
    for (auto children = vertex.children; children; children = children->next) {
      os << ' ' << children->vertex->index();
    }
    return os;
  }

private:
  static constinit const vertex *const vertex_begin;
  static constinit const stack_frame *const low_link_begin;
};

static constinit std::array<vertex, N> vertices{};
constinit const vertex *const vertex::vertex_begin = vertices.cbegin();
static constinit std::array<stack_frame, N> dfs_stack{};
constinit const stack_frame *const vertex::low_link_begin = dfs_stack.cbegin();

void preallocated(std::span<vertex> vertices, std::ranges::input_range auto &&edges) {
  for (const auto [source, target] : edges) {
    vertices[source].add_child(vertices[target]);
  }

  std::vector<std::ptrdiff_t> reverse_component_sizes;
  auto sorted = dfs_stack.begin() + vertices.size();
  for (auto top = dfs_stack.begin(); auto &vertex : vertices) {
    assert(top == dfs_stack.begin());
    // __builtin_debugtrap();
    if (vertex.visited()) {
      continue;
    }
    for (auto curr = vertex.stack(nullptr, top);;) {
#ifndef NDEBUG
      std::clog << **curr << std::endl;
#endif
      while (auto vertex = (*curr)->consume_child()) {
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
        std::clog << std::endl;
#endif
        if (!vertex::go_back(curr)) {
          break;
        }
      } else {
        auto vertex = *curr;
        vertex::go_back(curr);
        assert(curr);
        (*curr)->update_low_link(vertex);
      }
    end:
    }
  }
  assert(sorted == dfs_stack.begin());

  std::cout << reverse_component_sizes.size() << '\n';
  for (auto component_size : std::views::reverse(reverse_component_sizes)) {
    std::cout << component_size;
    while (component_size--) {
      std::cout << ' ' << (*sorted++)->index();
    }
    std::cout << '\n';
  }
}

namespace std {
template <typename CharT, typename Traits, typename A, typename B>
auto &operator>>(std::basic_istream<CharT, Traits> &is, std::pair<A, B> &pair) {
  return is >> pair.first >> pair.second;
}
} // namespace std

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  std::istream_iterator<std::pair<int, int>> pairs(std::cin);
  const auto [vertex_count, edge_count] = *pairs++;
  preallocated(std::span(vertices.begin(), vertex_count), std::views::counted(pairs, edge_count));
}
