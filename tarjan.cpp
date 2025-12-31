// https://judge.yosupo.jp/problem/scc
#include <algorithm>
#include <iostream>
#include <vector>

class Tarjan {
  const std::vector<std::vector<int>> &adj;
  std::vector<int> s, top, index;
  size_t mSize = 0;
  std::vector<int>::iterator si;

public:
  Tarjan(const std::vector<std::vector<int>> &adj)
      : adj(adj), index(adj.size()), s(adj.size()), si(s.begin()) {
    std::ranges::fill(index, -1);
    top.push_back(0);
    for (int i = 0; i != adj.size(); ++i) {
      tarjan(i);
    }
  }

  int tarjan(int i) {
    if (index[i] != -1) {
      return index[i];
    }
    int lowLink = index[i] = si - s.begin();
    *si++ = i;
    for (const int j : adj[i]) {
      lowLink = std::min(lowLink, tarjan(j));
    }
    if (lowLink == index[i]) {
      ++mSize;
      std::ranges::subrange scc(s.begin() + index[i], si);
      for (const int j : scc) {
        top.push_back(j);
        index[j] = adj.size();
      }
      top.push_back(-scc.size());
      si = scc.begin();
    }
    return lowLink;
  };

  struct iterator {
    struct value_type {
      using iterator = std::vector<int>::const_reverse_iterator;
      value_type(iterator first) : first(next(first)), last(next(first) - *first) {}
      iterator begin() const noexcept { return first; }
      iterator end() const noexcept { return last; }
      bool operator==(const value_type &) const noexcept = default;

    private:
      iterator first, last;
    };

    iterator(value_type::iterator first) : r(first) {}
    value_type operator*() const { return r; }
    iterator &operator++() {
      r = {r.end()};
      return *this;
    }
    bool operator==(const iterator &) const noexcept = default;

  private:
    value_type r;
  };
  iterator begin() const noexcept { return {top.rbegin()}; }
  iterator end() const noexcept { return {std::prev(top.rend())}; }
  size_t size() const noexcept { return mSize; }
};

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  int vertexCount, edgeCount;
  std::cin >> vertexCount >> edgeCount;
  std::vector<std::vector<int>> adj(vertexCount);
  for (int source, target; std::cin >> source >> target;) {
    adj[source].push_back(target);
  }
  Tarjan tarjan(adj);
  std::cout << tarjan.size() << '\n';
  for (const auto &scc : tarjan) {
    std::cout << std::ranges::size(scc);
    for (const int i : scc) {
      std::cout << ' ' << i;
    }
    std::cout << '\n';
  }
}
