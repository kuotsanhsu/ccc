// https://judge.yosupo.jp/problem/scc
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

struct vertex;
struct children {
  const children *next;
  vertex *vertex;
};
using stack_frame = vertex *;
class vertex {
  const children *children;
  const stack_frame *low_link;
  stack_frame *prev;

public:
  stack_frame *stack(stack_frame *prev, stack_frame *&top) {
    prev = prev;
    low_link = top;
    *top = this;
    return top++;
  }
  [[nodiscard]] bool visited() const noexcept { return low_link; }
  bool same_low_link(const stack_frame *curr) { return low_link == curr; }
  void update_low_link(const vertex *vertex) { low_link = std::min(low_link, vertex->low_link); }
  void add_child(struct children *&edge_pool_ptr, vertex &child) {
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
};

class strong_component_view : public std::ranges::view_interface<strong_component_view> {
  std::span<const stack_frame> sorted;
  std::span<const std::ptrdiff_t> reverse_component_sizes;

public:
  class iterator {
    std::span<const stack_frame>::iterator acc;
    std::span<const std::ptrdiff_t>::reverse_iterator curr;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::ranges::subrange<decltype(acc)>;

    constexpr iterator(std::span<const stack_frame>::iterator acc,
                       std::span<const std::ptrdiff_t>::reverse_iterator first) noexcept
        : acc(acc), curr(first) {}
    constexpr value_type operator*() const noexcept { return {acc, acc + *curr}; }
    constexpr iterator &operator++() {
      acc += *curr++;
      return *this;
    }
    constexpr iterator operator++(int) {
      auto old = *this;
      ++*this;
      return old;
    }
    constexpr bool operator==(std::span<const stack_frame>::iterator last) const noexcept {
      return acc == last;
    };
  };

  [[nodiscard]] constexpr iterator begin() const noexcept {
    return {sorted.begin(), reverse_component_sizes.rbegin()};
  }
  [[nodiscard]] constexpr auto end() const noexcept { return sorted.end(); }
  [[nodiscard]] constexpr size_t size() const noexcept { return reverse_component_sizes.size(); }

  constexpr strong_component_view(std::span<stack_frame> sorted,
                                  std::span<const std::ptrdiff_t> reverse_component_sizes)
      : sorted(sorted), reverse_component_sizes(reverse_component_sizes) {}
};

static_assert(std::ranges::input_range<strong_component_view>);
static_assert(std::input_iterator<strong_component_view::iterator>);
static_assert(std::ranges::input_range<strong_component_view::iterator::value_type>);

strong_component_view preallocated(std::span<vertex> vertices,
                                   std::ranges::input_range auto &&edges) {
  // log2(500'000) is about 18.897
  constexpr auto N = 500'000;
  constexpr auto M = 500'000;

  static children edge_pool[M];
  for (auto edge_pool_ptr = edge_pool; const auto [source, target] : edges) {
    vertices[source].add_child(edge_pool_ptr, vertices[target]);
  }

  std::vector<std::ptrdiff_t> reverse_component_sizes;
  static stack_frame stack[N];
  auto sorted = stack + vertices.size();
  for (auto top = stack; auto &vertex : vertices) {
    assert(top == stack);
    if (vertex.visited()) {
      continue;
    }
    for (auto curr = vertex.stack(nullptr, top);;) {
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
        while (top != curr) {
          *--sorted = *--top;
        }
        if (!vertex::go_back(curr)) {
          break;
        }
      } else {
        auto vertex = *curr;
        (*vertex::go_back(curr))->update_low_link(vertex);
      }
    end:
    }
  }
  assert(sorted == stack);
  return {std::span(sorted, vertices.size()), reverse_component_sizes};
}

namespace std {
template <typename CharT, typename Traits, typename A, typename B>
auto &operator>>(std::basic_istream<CharT, Traits> &is, std::pair<A, B> &pair) {
  return is >> pair.first >> pair.second;
}
} // namespace std

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  auto pairs = std::istream_iterator<std::pair<int, int>>(std::cin);
  const auto [vertex_count, edge_count] = *pairs++;
  std::vector<vertex> vertices(vertex_count);
  const auto strong_components = preallocated(vertices, std::views::counted(pairs, edge_count));

  const auto j = vertices.data();
  std::cout << std::ranges::size(strong_components) << '\n';
  for (const auto &component : strong_components) {
    std::cout << std::ranges::size(component);
    for (const auto i : component) {
      std::cout << ' ' << &*i - j;
    }
    std::cout << '\n';
  }
}
