// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <forward_list>
#include <iostream>
#include <memory_resource>
#include <ranges>
#include <vector>

struct vertex;

class state {
  static state *top;

  const state *low_link;
  vertex *parent;

  constexpr state(vertex *parent) noexcept : low_link(this), parent(parent) {}

public:
  static constexpr state *push_after(vertex *parent) noexcept { return new (top++) state(parent); }

  [[nodiscard]] constexpr const state *get_low_link() const noexcept { return low_link; }
  constexpr void set_low_link(const state *link) noexcept { low_link = std::min(low_link, link); }

  /// Effectively returns `parent`.
  [[nodiscard]] constexpr vertex *destruct(vertex *vertex) noexcept {
    return std::exchange(parent, vertex);
  }
  /// `pop_after` must be called after `destruct`.
  static constexpr std::ranges::sized_range auto pop_after(state *link) noexcept {
    return std::ranges::subrange(link, std::exchange(top, link)) |
           std::views::transform(&state::parent);
  }
};

using strong_component = std::invoke_result_t<decltype(state::pop_after), state *>;

class vertex {
  static state *const max_link;

  state *link;
  std::pmr::forward_list<std::reference_wrapper<vertex>> children;

public:
  constexpr ~vertex() noexcept { link = max_link; }

  [[nodiscard]] constexpr std::pair<vertex *, strong_component> component() noexcept {
    while (!children.empty()) {
      vertex &child = children.front();
      children.pop_front();
      if (child.link == nullptr) {
        child.link = state::push_after(this);
        __attribute__((musttail)) return child.component();
      }
      link->set_low_link(child.link);
    }
    const auto parent = link->destruct(this);
    const auto low_link = link->get_low_link();
    if (link == low_link) {
      return {parent, state::pop_after(link)};
    }
    parent->link->set_low_link(low_link);
    __attribute__((musttail)) return parent->component();
  }

  constexpr void add_child(vertex &child) noexcept { children.emplace_front(child); }

  [[nodiscard]] constexpr vertex *start() noexcept {
    if (link == nullptr) {
      link = state::push_after(nullptr);
      return this;
    }
    return nullptr;
  }
};

class strong_component_iterator {
public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = strong_component;

  constexpr bool operator==(std::default_sentinel_t /*unused*/) const noexcept {
    return curr == nullptr;
  }

  constexpr strong_component_iterator &operator++() noexcept { return *this; }
  constexpr strong_component_iterator operator++(int) noexcept {
    auto old = *this;
    ++*this;
    return old;
  }
  constexpr value_type operator*() const noexcept {
    const auto [parent, component] = curr->component();
    curr = parent;
    return component;
  }

  constexpr strong_component_iterator(vertex &curr) noexcept : curr(curr.start()) {}

private:
  mutable vertex *curr;
};

constexpr auto components(vertex &vertex) noexcept {
  return std::ranges::subrange(strong_component_iterator(vertex), std::default_sentinel);
}

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;

// static_assert(std::input_iterator<strong_component_iterator>);
static_assert(sizeof(state) == sizeof(std::ptrdiff_t) * 2);
static_assert(alignof(state) == alignof(std::ptrdiff_t));
std::array<std::ptrdiff_t, vertex_count_max << 1> indices{};
state *state::top = reinterpret_cast<state *>(indices.begin());
state *const vertex::max_link = reinterpret_cast<state *>(indices.end());

namespace {
struct unit {
  vertex v;
  static_assert(sizeof(v) == 24);
  struct forward_list_node {
    std::reference_wrapper<vertex> child;
    forward_list_node *next;
  } node;
  static_assert(sizeof(node) == 16);
};
static_assert(alignof(unit) == 8);

[[nodiscard]] constexpr std::size_t graph_size(std::size_t vertex_count,
                                               std::size_t edge_count) noexcept {
  static_assert(sizeof(vertex) == 24);
  // using T = decltype(vertex::children)::value_type;
  static_assert(sizeof(std::reference_wrapper<vertex>) == 8);
  static_assert(sizeof(void *) == 8);
  return vertex_count * sizeof(unit::v) + edge_count * sizeof(unit::forward_list_node);
}

void preallocated(std::size_t vertex_count, std::ranges::input_range auto &&edges) {
  alignas(unit) static std::array<std::byte, graph_size(vertex_count_max, edge_count_max)> raw;
  std::pmr::monotonic_buffer_resource pool(raw.data(),
                                           graph_size(vertex_count, std::ranges::size(edges)),
                                           std::pmr::null_memory_resource());
  std::pmr::vector<vertex> vertices(vertex_count, &pool);
  for (const auto [source, target] : edges) {
    vertices[source].add_child(vertices[target]);
  }
  const vertex *const vertex_begin = vertices.begin().base();

  std::ptrdiff_t *index_iter = indices.end();
  std::size_t strong_component_count = 0;
  for (auto strong_component : vertices | std::views::transform(components) | std::views::join) {
    for (const auto vertex : strong_component | std::views::reverse) {
      *--index_iter = vertex - vertex_begin;
      vertex->~vertex();
    }
    *--index_iter = std::ranges::size(strong_component);
    ++strong_component_count;
  }

  std::cout << strong_component_count << '\n';
  while (strong_component_count--) {
    auto strong_component_size = *index_iter++;
    std::cout << strong_component_size;
    while (strong_component_size--) {
      std::cout << ' ' << *index_iter++;
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

/*
 * [Packed bit fields in c structures - GCC](https://stackoverflow.com/q/25822679/16371358)
 * Reasons not to use packed bit fields: taking the address of bit fields is UB, ABI is not stable
 * across compilers, and endianness is troublesome.
 */
struct __attribute__((packed)) t1 {
  int a : 12;
  int b : 32;
  int c : 4;
};
static_assert(sizeof(t1) == 6);

struct __attribute__((packed)) t2 {
  int a : 12;
  int b;
  int c : 4;
};
static_assert(sizeof(t2) == 7);

struct __attribute__((packed)) t3 {
  _BitInt(12) a;
  int b;
  int c : 4;
};
static_assert(sizeof(t3) == 7);

struct __attribute__((packed)) t4 {
  _BitInt(12) a;
  _BitInt(32) b;
  _BitInt(4) c;
};
static_assert(sizeof(t4) == 7);

struct __attribute__((packed)) t5 {
  _BitInt(12) a : 12;
  _BitInt(32) b : 32;
  _BitInt(4) c : 4;
};
static_assert(sizeof(t5) == 6);
