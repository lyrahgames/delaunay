#pragma once
#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lyrahgames::delaunay {

struct point {
  float x, y;
};

constexpr auto circumcircle_intersection(const point* a,  //
                                         const point* b,  //
                                         const point* c,  //
                                         const point* p) noexcept {
  const auto axdx = a->x - p->x;
  const auto aydy = a->y - p->y;
  const auto bxdx = b->x - p->x;
  const auto bydy = b->y - p->y;
  const auto cxdx = c->x - p->x;
  const auto cydy = c->y - p->y;
  const auto sqsum_a = axdx * axdx + aydy * aydy;
  const auto sqsum_b = bxdx * bxdx + bydy * bydy;
  const auto sqsum_c = cxdx * cxdx + cydy * cydy;
  const auto det = axdx * (bydy * sqsum_c - cydy * sqsum_b) -
                   aydy * (bxdx * sqsum_c - cxdx * sqsum_b) +
                   sqsum_a * (bxdx * cydy - cxdx * bydy);
  const point edge1{b->x - a->x, b->y - a->y};
  const point edge2{c->x - a->x, c->y - a->y};
  const auto d = (edge1.x * edge2.y - edge1.y * edge2.x);
  return (d * det) > 0.0f;
};

struct facet : public std::array<size_t, 2> {
  using base_type = std::array<size_t, 2>;

  struct hash {
    constexpr size_t operator()(const facet& e) const noexcept {
      return e[0] ^ (e[1] << 3);
    }
  };

  facet(size_t a, size_t b) : base_type{std::min(a, b), std::max(a, b)} {}
};

struct simplex : public std::array<size_t, 3> {
  using base_type = std::array<size_t, 3>;

  struct hash {
    constexpr size_t operator()(const simplex& t) const noexcept {
      return t[0] ^ (t[1] << 3) ^ (t[2] << 5);
    }
  };

  simplex(size_t a, size_t b, size_t c) : base_type{a, b, c} {
    std::sort(begin(), end());
  }
};

std::vector<simplex> triangulation(std::vector<point>& points) {
  // Construct much larger bounding box for all points.
  constexpr point bounds[4] = {
      {-1.0e6f, -1.0e6f},
      {1.0e6f, -1.0e6f},
      {1.0e6f, 1.0e6f},
      {-1.0e6f, 1.0e6f},
  };
  std::unordered_set<simplex, simplex::hash> simplices{
      {reinterpret_cast<size_t>(&bounds[0]),  //
       reinterpret_cast<size_t>(&bounds[1]),  //
       reinterpret_cast<size_t>(&bounds[2])},
      {reinterpret_cast<size_t>(&bounds[2]),  //
       reinterpret_cast<size_t>(&bounds[3]),  //
       reinterpret_cast<size_t>(&bounds[0])}};

  std::unordered_map<facet, int, facet::hash> polytope{};

  // Incrementally insert every point.
  for (const auto& p : points) {
    // Construct the polytope for a new polytope
    // according to Bowyer and Watson.
    polytope.clear();
    // Test for the circumcircle intersection with every polytope.
    for (auto it = simplices.begin(); it != simplices.end();) {
      auto& t = *it;
      if (circumcircle_intersection(reinterpret_cast<const point*>(t[0]),
                                    reinterpret_cast<const point*>(t[1]),
                                    reinterpret_cast<const point*>(t[2]), &p)) {
        // If so, simplex has to be removed and added to the polytope.
        ++polytope[{t[0], t[1]}];
        ++polytope[{t[1], t[2]}];
        ++polytope[{t[2], t[0]}];
        it = simplices.erase(it);
      } else {
        ++it;
      }
    }
    // Add new simplices by connecting boundary facets
    // of the polytope with the new point.
    for (auto& [e, i] : polytope)
      if (i == 1) simplices.insert({e[0], e[1], reinterpret_cast<size_t>(&p)});
  }

  // Construct the result vector by adding all simplices
  // not referencing points of the bounding box.
  std::vector<simplex> result{};
  result.reserve(simplices.size());
  for (const auto& t : simplices) {
    const auto a =
        static_cast<size_t>(reinterpret_cast<const point*>(t[0]) - &points[0]);
    const auto b =
        static_cast<size_t>(reinterpret_cast<const point*>(t[1]) - &points[0]);
    const auto c =
        static_cast<size_t>(reinterpret_cast<const point*>(t[2]) - &points[0]);

    if ((a < points.size()) && (b < points.size()) && (c < points.size()))
      result.emplace_back(a, b, c);
  }

  return result;
}

}  // namespace lyrahgames::delaunay