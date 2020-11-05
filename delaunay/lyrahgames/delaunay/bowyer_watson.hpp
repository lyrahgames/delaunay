#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <vector>

namespace lyrahgames::delaunay::bowyer_watson {

struct point {
  float x, y;
};

constexpr point operator+(point x, point y) noexcept {
  return {x.x + y.x, x.y + y.y};
}

constexpr point operator-(point x, point y) noexcept {
  return {x.x - y.x, x.y - y.y};
}

constexpr point operator*(float x, point v) noexcept {
  return {x * v.x, x * v.y};
}

constexpr auto dot(point x, point y) noexcept { return x.x * y.x + x.y * y.y; }

constexpr auto sqnorm(point x) noexcept { return dot(x, x); }

constexpr auto crossdot(point x, point y) noexcept {
  return x.x * y.y - x.y * y.x;
}

constexpr auto circumcircle_intersection(const point& a,  //
                                         const point& b,  //
                                         const point& c,  //
                                         const point& p) noexcept {
  const auto axdx = a.x - p.x;
  const auto aydy = a.y - p.y;
  const auto bxdx = b.x - p.x;
  const auto bydy = b.y - p.y;
  const auto cxdx = c.x - p.x;
  const auto cydy = c.y - p.y;
  const auto sqsum_a = axdx * axdx + aydy * aydy;
  const auto sqsum_b = bxdx * bxdx + bydy * bydy;
  const auto sqsum_c = cxdx * cxdx + cydy * cydy;
  const auto det = axdx * (bydy * sqsum_c - cydy * sqsum_b) -
                   aydy * (bxdx * sqsum_c - cxdx * sqsum_b) +
                   sqsum_a * (bxdx * cydy - cxdx * bydy);
  const point edge1{b.x - a.x, b.y - a.y};
  const point edge2{c.x - a.x, c.y - a.y};
  const auto d = (edge1.x * edge2.y - edge1.y * edge2.x);
  return (d * det) > 0.0f;
};

// constexpr auto circumcircle_intersection_cache(const point& a, const point&
// b,
//                                                const point& c) noexcept {
//   const auto u = b - a;
//   const auto v = c - a;
//   const auto u2 = sqnorm(u);
//   const auto v2 = sqnorm(v);
//   const result[3] = {u.y * v2 - v.y * u2, v.x * u2 - u.x * v2,
//                      u.x * v.y - u.y * v.x};
// }

struct edge : public std::array<size_t, 2> {
  using base_type = std::array<size_t, 2>;
  edge(size_t a, size_t b) : base_type{std::min(a, b), std::max(a, b)} {}
};

struct triangle : public std::array<size_t, 3> {
  using base_type = std::array<size_t, 3>;
  triangle(size_t a, size_t b, size_t c) : base_type{a, b, c} {
    std::sort(begin(), end());
  }
};

std::vector<triangle> triangulation(std::vector<point>& points) {
  const point bounds[4] = {
      {-1.0e6f, -1.0e6f},
      {1.0e6f, -1.0e6f},
      {1.0e6f, 1.0e6f},
      {-1.0e6f, 1.0e6f},
  };

  std::vector<triangle> triangles{{reinterpret_cast<size_t>(&bounds[0]),  //
                                   reinterpret_cast<size_t>(&bounds[1]),  //
                                   reinterpret_cast<size_t>(&bounds[2])},
                                  {reinterpret_cast<size_t>(&bounds[2]),  //
                                   reinterpret_cast<size_t>(&bounds[3]),  //
                                   reinterpret_cast<size_t>(&bounds[0])}};

  std::map<edge, int> polygon{};
  std::vector<size_t> bad_triangles{};

  for (const auto& p : points) {
    polygon.clear();
    bad_triangles.clear();

    for (size_t i = 0; i < triangles.size(); ++i) {
      auto& t = triangles[i];
      if (circumcircle_intersection(*reinterpret_cast<const point*>(t[0]),
                                    *reinterpret_cast<const point*>(t[1]),
                                    *reinterpret_cast<const point*>(t[2]), p)) {
        bad_triangles.push_back(i);
        ++polygon[{t[0], t[1]}];
        ++polygon[{t[1], t[2]}];
        ++polygon[{t[2], t[0]}];
      }
    }

    size_t bad_id = 0;
    auto edge_it = polygon.begin();

    for (; bad_id < bad_triangles.size(); ++bad_id, ++edge_it) {
      while (edge_it->second != 1) ++edge_it;
      auto& [e, i] = *edge_it;
      triangles[bad_triangles[bad_id]] =
          triangle{e[0], e[1], reinterpret_cast<size_t>(&p)};
    }

    for (; edge_it != polygon.end(); ++edge_it) {
      auto& [e, i] = *edge_it;
      if (i != 1) continue;
      triangles.push_back(triangle{e[0], e[1], reinterpret_cast<size_t>(&p)});
    }
  }

  std::vector<triangle> result{};
  result.reserve(triangles.size());
  for (const auto& t : triangles) {
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

}  // namespace lyrahgames::delaunay::bowyer_watson