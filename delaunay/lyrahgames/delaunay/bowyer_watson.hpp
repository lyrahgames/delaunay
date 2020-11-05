#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
//
#include <lyrahgames/delaunay/geometry.hpp>

namespace lyrahgames::delaunay::bowyer_watson {

using point = float32x2;

struct edge : public std::array<size_t, 2> {
  using base_type = std::array<size_t, 2>;
  struct hash {
    constexpr size_t operator()(const edge& e) const noexcept {
      return e[0] ^ (e[1] << 3);
    }
  };
  edge(size_t a, size_t b) : base_type{std::min(a, b), std::max(a, b)} {}
};

struct triangle : public std::array<size_t, 3> {
  using base_type = std::array<size_t, 3>;
  triangle(size_t a, size_t b, size_t c) : base_type{a, b, c} {
    std::sort(begin(), end());
  }
};

std::vector<triangle> triangulation(std::vector<point>& points) {
  // Construct regular super triangle which contains all given points.
  const auto bounds = bounding_triangle(bounding_circle(bounding_box(points)));
  std::vector<triangle> triangles{
      {reinterpret_cast<size_t>(&bounds[0]),  //
       reinterpret_cast<size_t>(&bounds[1]),  //
       reinterpret_cast<size_t>(&bounds[2])},
  };

  // const point bounds[4] = {
  //     {-1.0e6f, -1.0e6f},
  //     {1.0e6f, -1.0e6f},
  //     {1.0e6f, 1.0e6f},
  //     {-1.0e6f, 1.0e6f},
  // };
  // std::vector<triangle> triangles{{reinterpret_cast<size_t>(&bounds[0]),  //
  //                                  reinterpret_cast<size_t>(&bounds[1]),  //
  //                                  reinterpret_cast<size_t>(&bounds[2])},
  //                                 {reinterpret_cast<size_t>(&bounds[2]),  //
  //                                  reinterpret_cast<size_t>(&bounds[3]),  //
  //                                  reinterpret_cast<size_t>(&bounds[0])}};

  // std::unordered_map<edge, int, edge::hash> polygon{};
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

namespace experimental {

std::vector<triangle> triangulation(std::vector<point>& points) {
  // Construct regular super triangle which contains all given points.
  const auto bounds = bounding_triangle(bounding_circle(bounding_box(points)));
  std::vector<triangle> triangles{
      {reinterpret_cast<size_t>(&bounds[0]),  //
       reinterpret_cast<size_t>(&bounds[1]),  //
       reinterpret_cast<size_t>(&bounds[2])},
  };
  // triangles.reserve(points.size() + 3);
  std::vector<std::array<float, 3>> cache{circumcircle_intersection_cache(
      *reinterpret_cast<const point*>(triangles[0][0]),
      *reinterpret_cast<const point*>(triangles[0][1]),
      *reinterpret_cast<const point*>(triangles[0][2]))};
  // cache.reserve(points.size() + 3);

  // std::unordered_map<edge, int, edge::hash> polygon(32);
  std::map<edge, int> polygon{};
  // std::set<edge> polygon{};
  // std::unordered_set<edge, edge::hash> polygon{};
  std::vector<size_t> bad_triangles(32);

  for (const auto& p : points) {
    polygon.clear();
    bad_triangles.clear();

    for (size_t i = 0; i < triangles.size(); ++i) {
      auto& t = triangles[i];
      if (circumcircle_intersection(*reinterpret_cast<const point*>(t[0]),
                                    cache[i], p)) {
        bad_triangles.push_back(i);

        ++polygon[{t[0], t[1]}];
        ++polygon[{t[1], t[2]}];
        ++polygon[{t[2], t[0]}];
        // {
        //   auto [it, inserted] = polygon.insert({t[0], t[1]});
        //   if (!inserted) polygon.erase(it);
        // }
        // {
        //   auto [it, inserted] = polygon.insert({t[1], t[2]});
        //   if (!inserted) polygon.erase(it);
        // }
        // {
        //   auto [it, inserted] = polygon.insert({t[2], t[0]});
        //   if (!inserted) polygon.erase(it);
        // }
      }
    }

    size_t bad_id = 0;
    auto edge_it = polygon.begin();

    for (; bad_id < bad_triangles.size(); ++bad_id, ++edge_it) {
      while (edge_it->second != 1) ++edge_it;
      auto& [e, i] = *edge_it;
      // auto& e = *edge_it;
      triangles[bad_triangles[bad_id]] =
          triangle{e[0], e[1], reinterpret_cast<size_t>(&p)};
      cache[bad_triangles[bad_id]] = circumcircle_intersection_cache(
          *reinterpret_cast<const point*>(triangles[bad_triangles[bad_id]][0]),
          *reinterpret_cast<const point*>(triangles[bad_triangles[bad_id]][1]),
          *reinterpret_cast<const point*>(triangles[bad_triangles[bad_id]][2]));
    }

    for (; edge_it != polygon.end(); ++edge_it) {
      auto& [e, i] = *edge_it;
      if (i != 1) continue;
      // auto& e = *edge_it;
      triangles.push_back(triangle{e[0], e[1], reinterpret_cast<size_t>(&p)});
      cache.push_back(circumcircle_intersection_cache(
          *reinterpret_cast<const point*>(triangles.back()[0]),
          *reinterpret_cast<const point*>(triangles.back()[1]),
          *reinterpret_cast<const point*>(triangles.back()[2])));
    }
  }

  // Construct the result vector by adding all triangles
  // not referencing points of the bounding box.
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

}  // namespace experimental

}  // namespace lyrahgames::delaunay::bowyer_watson