#pragma once
#include <algorithm>
#include <numeric>
#include <ranges>
#include <vector>
//
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/delaunay/geometry.hpp>

namespace lyrahgames::delaunay {

namespace guibas_stolfi {

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

  edge_algebra(const std::vector<point>& data) : points(data) {
    std::ranges::sort(points, [](const auto& x, const auto& y) {
      return (x[0] < y[0]) || ((x[0] == y[0]) && (x[1] <= y[1]));
    });

    auto [l, r] = triangulate(0, points.size());

    // convex hull face
    data_id_t face = faces.size();
    //
    auto e = r;
    auto min_id = e;
    do {
      set_lface(e, face);
      e = lnext(e);
      min_id = std::min(min_id, e);
    } while (e != r);
    faces.push_back(min_id);

    for (edge_id_t edge = 0; edge < 4 * quad_edges.size(); edge += 2) {
      if (onode(edge) == dnode(edge))
        continue;
      if (lface(edge) != -1)
        continue;
      data_id_t face = faces.size();
      faces.push_back(edge);
      auto e = edge;
      do {
        set_lface(e, face);
        e = lnext(e);
      } while (e != edge);
    }

    using namespace std;
    for (size_t c = 0; auto& e : quad_edges) {
      for (size_t i = 0; i < 4; ++i)
        cout << setw(8) << c++;
      cout << '\n';
      for (size_t i = 0; i < 4; ++i)
        cout << setw(8) << e[i].next;
      cout << '\n';
      for (size_t i = 0; i < 4; ++i)
        cout << setw(8) << int(e[i].data);
      cout << '\n' << endl;
    }

    for (size_t i = 0; auto face : faces) {
      cout << setw(8) << i;
      ++i;
    }
    cout << '\n';
    for (auto face : faces)
      cout << setw(8) << face;
    cout << '\n';

    cout << setw(15) << "points = " << setw(15) << points.size() << '\n'
         << setw(15) << "edges = " << setw(15) << quad_edges.size() << '\n'
         << setw(15) << "faces = " << setw(15) << faces.size() << '\n'
         << setw(15) << "deleted edges = " << setw(15) << free_edges.size()
         << '\n';
  }

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

  bool ccw(data_id_t i, data_id_t j, data_id_t k) const {
    return counterclockwise(points[i], points[j], points[k]);
  }

  bool right_of(data_id_t i, edge_id_t e) const {
    return ccw(i, dnode(e), onode(e));
  }

  bool left_of(data_id_t i, edge_id_t e) const {
    return ccw(i, onode(e), dnode(e));
  }

  bool in_circle(data_id_t i, data_id_t j, data_id_t k, data_id_t l) const {
    return circumcircle_intersection(points[i], points[j], points[k],
                                     points[l]);
  }

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
    set_onode(e, 0);
    set_dnode(e, 0);
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

  auto triangulate(size_t first, size_t last)
      -> std::pair<edge_id_t, edge_id_t> {
    const auto size = last - first;

    if (size == 2) {
      auto a = new_edge();
      set_onode(a, first);
      set_dnode(a, first + 1);
      return {a, sym(a)};
    }

    if (size == 3) {
      auto a = new_edge();
      auto b = new_edge();
      splice(sym(a), b);
      set_onode(a, first + 0);
      set_dnode(a, first + 1);
      set_onode(b, first + 1);
      set_dnode(b, first + 2);

      if (ccw(first + 0, first + 1, first + 2)) {
        auto c = connection(b, a);
        return {a, sym(b)};
      } else if (ccw(first + 0, first + 2, first + 1)) {
        auto c = connection(b, a);
        return {sym(c), c};
      } else
        return {a, sym(b)};
    }

    const auto half = first + size / 2;
    auto [ldo, ldi] = triangulate(first, half);
    auto [rdi, rdo] = triangulate(half, last);

    while (true) {
      if (left_of(onode(rdi), ldi))
        ldi = lnext(ldi);
      else if (right_of(onode(ldi), rdi))
        rdi = rprev(rdi);
      else
        break;
    }

    auto baseline = connection(sym(rdi), ldi);
    if (onode(ldi) == onode(ldo))
      ldo = sym(baseline);
    if (onode(rdi) == onode(rdo))
      rdo = baseline;

    while (true) {
      auto lcand = onext(sym(baseline));
      auto rcand = oprev(baseline);

      const auto valid = [&](auto x) { return right_of(dnode(x), baseline); };

      auto lcand_valid = right_of(dnode(lcand), baseline);
      auto rcand_valid = right_of(dnode(rcand), baseline);

      if (valid(lcand)) {
        while (valid(onext(lcand)) &&
               in_circle(dnode(baseline), onode(baseline), dnode(lcand),
                         dnode(onext(lcand)))) {
          // (dnode(onext(lcand)) != onode(baseline)) &&
          auto t = onext(lcand);
          remove(lcand);
          lcand = t;
        }
      }

      if (valid(rcand)) {
        while (valid(oprev(rcand)) &&
               in_circle(dnode(baseline), onode(baseline), dnode(rcand),
                         dnode(oprev(rcand)))) {
          auto t = oprev(rcand);
          remove(rcand);
          rcand = t;
        }
      }

      if ((!valid(lcand)) && (!valid(rcand)))
        break;

      if (!valid(lcand) ||
          (valid(rcand) &&
           in_circle(dnode(lcand), onode(lcand), onode(rcand), dnode(rcand))))
        baseline = connection(rcand, sym(baseline));
      else
        baseline = connection(sym(baseline), sym(lcand));
    }

    return {ldo, rdo};
  }

 public:
  std::vector<point> points{};
  std::vector<quad_edge> quad_edges{};
  std::vector<edge_id_t> faces{};
  std::vector<edge_id_t> free_edges{};
};

class triangulator : public edge_algebra {
 public:
  triangulator() = default;

 public:
  std::vector<point> vertices{};
  std::vector<edge_id_t> faces{};
};

}  // namespace guibas_stolfi

}  // namespace lyrahgames::delaunay