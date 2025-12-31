// https://judge.yosupo.jp/problem/scc
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

class graph {
  std::vector<std::vector<std::size_t>> vertices;

public:
  constexpr graph(std::size_t vertex_count) : vertices(vertex_count) {}
  constexpr void push_edge(std::size_t source, std::size_t target) {
    vertices[source].push_back(target);
  }
  [[nodiscard]] constexpr auto vertex_count() const noexcept { return vertices.size(); }

  class strong_component_view : public std::ranges::view_interface<strong_component_view> {
    std::vector<std::size_t> sorted, reverse_component_sizes;
    using link = decltype(sorted)::iterator;

  public:
    class iterator {
      std::vector<std::size_t>::const_iterator acc;
      std::vector<std::size_t>::const_reverse_iterator curr;

    public:
      using difference_type = std::ptrdiff_t;
      using value_type = std::ranges::subrange<decltype(acc)>;

      constexpr iterator(decltype(acc) acc, decltype(curr) first) noexcept
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
      constexpr bool operator==(std::vector<std::size_t>::const_iterator last) const noexcept {
        return acc == last;
      };
    };

    [[nodiscard]] constexpr iterator begin() const {
      return {sorted.begin(), reverse_component_sizes.rbegin()};
    }
    [[nodiscard]] constexpr std::vector<std::size_t>::const_iterator end() const {
      return sorted.end();
    }
    [[nodiscard]] constexpr size_t size() const noexcept { return reverse_component_sizes.size(); }

    constexpr strong_component_view(const graph &);
  };
  [[nodiscard]] constexpr strong_component_view strong_components() const { return {*this}; }
};

static_assert(std::ranges::input_range<graph::strong_component_view>);
static_assert(std::input_iterator<graph::strong_component_view::iterator>);
static_assert(std::ranges::input_range<graph::strong_component_view::iterator::value_type>);

constexpr graph::strong_component_view::strong_component_view(const graph &graph)
    : sorted(graph.vertex_count()) {
  auto top = sorted.begin(), y = sorted.end();
  std::vector<link> links(sorted.size());
  // GCC has a bug that prevents "capturing this":
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=113563
  auto &reverse_component_sizes = this->reverse_component_sizes;
  const auto tarjan = [&graph, &top, &y, &links, &reverse_component_sizes](this auto &&tarjan,
                                                                           std::size_t i) {
    if (const auto link = links[i]; link.base()) {
      return link;
    }
    auto low_link = links[i] = top;
    *top++ = i;
    for (const auto j : graph.vertices[i]) {
      low_link = std::min(low_link, tarjan(j));
    }
    if (low_link == links[i]) {
      auto x = low_link;
      while (x != top && top != y) {
        *--y = *x++;
      }
      if (top == y) {
        std::reverse(x, top);
        y = x;
      }
      for (const auto i : std::ranges::subrange(low_link, top)) {
        links[i] = y;
      }
      reverse_component_sizes.push_back(top - low_link);
      top = low_link;
    }
    return low_link;
  };

  std::ranges::for_each(std::views::iota(0uz, graph.vertex_count()), tarjan);
  assert(top == sorted.begin());
  assert(y == sorted.begin());
}

std::ostream &operator<<(std::ostream &os, graph::strong_component_view strong_components) {
  os << std::ranges::size(strong_components) << '\n';
  for (const auto &component : strong_components) {
    os << std::ranges::size(component);
    for (const int i : component) {
      os << ' ' << i;
    }
    os << '\n';
  }
  return os;
}

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  int vertex_count, edge_count;
  std::cin >> vertex_count >> edge_count;
  graph graph(vertex_count);
  for (int source, target; std::cin >> source >> target;) {
    graph.push_edge(source, target);
  }
  std::cout << graph.strong_components();
}

template <typename T> class list {
  struct node {
    T value;
    node *next;
  };
  node *front = nullptr;

public:
  constexpr void push_front(const T &value) {
    node node{value, front};
    front = &node;
  }
  constexpr void push_front(T &&value) {
    node node{std::move(value), front};
    front = &node;
  }
};

static constexpr graph test(std::size_t vertex_count, std::ranges::input_range auto &&edges) {
  graph graph(vertex_count);
  for (const auto [source, target] : edges) {
    assert(source < vertex_count && target < vertex_count);
    graph.push_edge(source, target);
  }
  return graph;
}

static constexpr bool ranges_of_ranges_equal(auto &&r1, auto &&r2) {
  return std::ranges::equal(r1, r2,
                            [](auto &&r1, auto &&r2) { return std::ranges::equal(r1, r2); });
}

static_assert(ranges_of_ranges_equal(test(6, //
                                          std::array<std::pair<int, int>, 7>{{
                                              {1, 4},
                                              {5, 2},
                                              {3, 0},
                                              {5, 5},
                                              {4, 1},
                                              {0, 3},
                                              {4, 2},
                                          }})
                                         .strong_components(),
                                     std::initializer_list<std::initializer_list<int>>{
                                         {5},
                                         {4, 1},
                                         {2},
                                         {3, 0},
                                     }));
