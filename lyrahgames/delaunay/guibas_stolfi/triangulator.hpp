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
#include <lyrahgames/delaunay/guibas_stolfi/edge_algebra.hpp>

namespace lyrahgames::delaunay::guibas_stolfi {

class triangulator : public edge_algebra {
 public:
  triangulator(const std::vector<point>& data)
      : vertices(data), permutation(data.size()) {
    std::iota(permutation.begin(), permutation.end(), 0);
    std::ranges::sort(permutation, [&data](auto x, auto y) {
      return (data[x][0] < data[y][0]) ||
             ((data[x][0] == data[y][0]) && (data[x][1] <= data[y][1]));
    });
    for (size_t i = 0; i < data.size(); ++i) {
      vertices[i] = data[permutation[i]];
    }
    // std::ranges::sort(vertices, [](const auto& x, const auto& y) {
    //   return (x[0] < y[0]) || ((x[0] == y[0]) && (x[1] <= y[1]));
    // });

    auto [l, r] = triangulate(0, vertices.size());
    generate_faces(r);
  }

  bool ccw(data_id_t i, data_id_t j, data_id_t k) const {
    return counterclockwise(vertices[i], vertices[j], vertices[k]);
  }

  bool right_of(data_id_t i, edge_id_t e) const {
    return ccw(i, dnode(e), onode(e));
  }

  bool left_of(data_id_t i, edge_id_t e) const {
    return ccw(i, onode(e), dnode(e));
  }

  bool in_circle(data_id_t i, data_id_t j, data_id_t k, data_id_t l) const {
    return circumcircle_intersection(vertices[i], vertices[j], vertices[k],
                                     vertices[l]);
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

  void generate_faces(edge_id_t r) {
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
  }

  friend std::ostream& operator<<(std::ostream& os, const triangulator& t) {
    using namespace std;
    os << static_cast<const edge_algebra&>(t) << '\n';
    for (size_t i = 0; auto face : t.faces) {
      os << setw(8) << i;
      ++i;
    }
    os << '\n';
    for (auto face : t.faces)
      os << setw(8) << face;
    os << '\n';
    os << setw(15) << "vertices = " << setw(15) << t.vertices.size() << '\n'
       << setw(15) << "edges = " << setw(15) << t.quad_edges.size() << '\n'
       << setw(15) << "faces = " << setw(15) << t.faces.size() << '\n'
       << setw(15) << "deleted edges = " << setw(15) << t.free_edges.size()
       << '\n';
    return os;
  }

 public:
  std::vector<point> vertices{};
  std::vector<data_id_t> permutation{};
  std::vector<edge_id_t> faces{};
};

}  // namespace lyrahgames::delaunay::guibas_stolfi