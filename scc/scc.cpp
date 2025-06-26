#include <array>
#include <iostream>
#include <span>
#include <stack>
#include <tuple>
#include <vector>

constexpr std::vector<std::vector<int>> scc(const int N,
                                            const std::span<const std::pair<int, int>> edges) {
  // const auto M{edges.size()};
  struct Vertex {
    std::vector<Vertex *> successors{};
    int index{-1};
    int lowlink{-1};
    bool onStack{false};

    void visit(const int index) noexcept {
      lowlink = this->index = index;
      onStack = true;
    }

    [[nodiscard]] bool visited() const noexcept { return index != -1; }
  };
  std::vector<Vertex> vertices(N);
  const Vertex *const front = &vertices.front();
  for (const auto [from, to] : edges) {
    vertices[from].successors.push_back(&vertices[to]);
  }

  std::stack<Vertex *> S;
  std::vector<std::vector<int>> components;
  const auto strongconnect = [front, &vertices, &S, &components](this auto &&self,
                                                                 Vertex *const v) -> void {
    v->visit(S.size());
    S.push(v);
    for (auto w : v->successors) {
      if (!w->visited()) {
        // Successor w has not yet been visited; recurse on it.
        self(w);
        v->lowlink = std::min(v->lowlink, w->lowlink);
      } else if (w->onStack) {
        // Successor w is in stack S and hence in the current SCC. If w is not on stack, then (v, w)
        // is an edge pointing to an SCC already found and must be ignored.
        v->lowlink = std::min(v->lowlink, w->lowlink);
      }
    }
    // If v is a root node, pop the stack and generate an SCC.
    if (v->lowlink == v->index) {
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

int main() {
  constexpr int N{6}, M{7};
  constexpr std::array<std::pair<int, int>, M> edges{
      {{1, 4}, {5, 2}, {3, 0}, {5, 5}, {4, 1}, {0, 3}, {4, 2}},
  };
  constexpr int K{4};
  constexpr std::tuple components{
      std::array{5},
      std::array{4, 1},
      std::array{2},
      std::array{3, 0},
  };

  const auto result = scc(N, edges);
  std::cout << result.size() << '\n';
  for (const auto &component : result) {
    std::cout << component.size();
    for (const auto v : component) {
      std::cout << ' ' << v;
    }
    std::cout << '\n';
  }
}
