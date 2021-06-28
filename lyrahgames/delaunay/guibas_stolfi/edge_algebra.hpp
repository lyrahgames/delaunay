#pragma once
#include <iomanip>
#include <iostream>
#include <vector>

namespace lyrahgames::delaunay::guibas_stolfi {

class edge_algebra {
 public:
  using edge_id_t = size_t;
  using data_id_t = size_t;

  struct edge {
    edge_id_t next{};
    data_id_t data = -1;
  };

  struct quad_edge {
    edge& operator[](int index) { return data[index]; }
    const edge& operator[](int index) const { return data[index]; }

    edge data[4]{};
  };

  static constexpr edge_id_t edge_type_mask =
      sizeof(struct quad_edge) / sizeof(struct edge) - 1;
  static constexpr edge_id_t quad_edge_mask = ~edge_type_mask;

  edge_algebra() = default;

 public:
  auto edge_data() { return reinterpret_cast<edge*>(quad_edges.data()); }

  auto edge_data() const {
    return reinterpret_cast<const edge*>(quad_edges.data());
  }

  auto rot(edge_id_t e, int n = 1) const {
    const edge_id_t t = e + n;
    return static_cast<edge_id_t>((e & quad_edge_mask) | (t & edge_type_mask));
  }

  auto sym(edge_id_t e) const { return rot(e, 2); }

  auto onext(edge_id_t e) const { return (edge_data() + e)->next; }

  auto oprev(edge_id_t e) const { return rot(onext(rot(e))); }

  auto dnext(edge_id_t e) const { return sym(onext(sym(e))); }

  auto dprev(edge_id_t e) const { return rot(onext(rot(e, -1)), -1); }

  auto lnext(edge_id_t e) const -> edge_id_t { return rot(onext(rot(e, -1))); }

  auto lprev(edge_id_t e) const { return sym(onext(e)); }

  auto rnext(edge_id_t e) const { return rot(onext(rot(e)), -1); }

  auto rprev(edge_id_t e) const { return onext(sym(e)); }

  auto onode(edge_id_t e) const -> data_id_t { return (edge_data() + e)->data; }

  auto dnode(edge_id_t e) const -> data_id_t { return onode(sym(e)); }

  auto lface(edge_id_t e) const -> data_id_t { return onode(rot(e, -1)); }

  auto rface(edge_id_t e) const -> data_id_t { return onode(rot(e)); }

  void set_onode(edge_id_t e, data_id_t x) { (edge_data() + e)->data = x; }

  void set_dnode(edge_id_t e, data_id_t x) { set_onode(sym(e), x); }

  void set_lface(edge_id_t e, data_id_t x) { set_onode(rot(e, -1), x); }

  void set_rface(edge_id_t e, data_id_t x) { set_onode(rot(e), x); }

  auto quad_edge_index(edge_id_t e) const -> size_t { return e >> 2; }

  auto new_edge(size_t index) -> edge_id_t {
    constexpr size_t scale = sizeof(quad_edge) / sizeof(edge);
    const auto data = scale * index;
    auto& e = quad_edges[index];
    e[0].next = data + 0;
    e[1].next = data + 3;
    e[2].next = data + 2;
    e[3].next = data + 1;
    return data;
  }

  auto new_edge() -> edge_id_t {
    if (!free_edges.empty()) {
      const auto index = free_edges.back();
      free_edges.pop_back();
      return new_edge(index);
    }
    const auto index = quad_edges.size();
    quad_edges.push_back({});
    return new_edge(index);
  }

  void splice(edge_id_t a, edge_id_t b) {
    auto alpha = rot(onext(a));
    auto beta = rot(onext(b));
    auto t1 = onext(b);
    auto t2 = onext(a);
    auto t3 = onext(beta);
    auto t4 = onext(alpha);
    (edge_data() + a)->next = t1;
    (edge_data() + b)->next = t2;
    (edge_data() + alpha)->next = t3;
    (edge_data() + beta)->next = t4;
  }

  auto connection(edge_id_t a, edge_id_t b) -> edge_id_t {
    auto e = new_edge();
    set_onode(e, dnode(a));
    set_dnode(e, onode(b));
    splice(e, lnext(a));
    splice(sym(e), b);
    return e;
  }

  void remove(edge_id_t e) {
    splice(e, oprev(e));
    splice(sym(e), oprev(sym(e)));
    free_edges.push_back(quad_edge_index(e));
  }

  void swap(edge_id_t e) {
    auto a = oprev(e);
    auto b = oprev(sym(e));
    splice(e, a);
    splice(sym(e), b);
    splice(e, lnext(a));
    splice(sym(e), lnext(b));
    set_onode(e, dnode(a));
    set_dnode(e, dnode(b));
  }

  friend std::ostream& operator<<(std::ostream& os, const edge_algebra& ea) {
    using namespace std;
    for (size_t c = 0; auto& e : ea.quad_edges) {
      for (size_t i = 0; i < 4; ++i)
        os << setw(8) << c++;
      os << '\n';
      for (size_t i = 0; i < 4; ++i)
        os << setw(8) << e[i].next;
      os << '\n';
      for (size_t i = 0; i < 4; ++i)
        os << setw(8) << int(e[i].data);
      os << '\n' << endl;
    }
    return os;
  }

 public:
  std::vector<quad_edge> quad_edges{};
  std::vector<edge_id_t> free_edges{};
};

}  // namespace lyrahgames::delaunay::guibas_stolfi