#pragma once
#include <algorithm>
#include <array>
#include <map>
#include <vector>
// #include <set>
// #include <unordered_map>
// #include <unordered_set>
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
    // std::sort(begin(), end());
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

  // Initialize structures for the intersection polygon.
  std::map<edge, int> polygon{};
  // std::unordered_map<edge, int, edge::hash> polygon{};
  std::vector<size_t> bad_triangles{};

  // Incrementally insert every point.
  for (const auto& p : points) {
    // Clear the old polygon.
    polygon.clear();
    bad_triangles.clear();

    // Ask for the circumcircle intersection with all triangles.
    for (size_t i = 0; i < triangles.size(); ++i) {
      const auto& t = triangles[i];
      if (circumcircle_intersection(*reinterpret_cast<const point*>(t[0]),
                                    *reinterpret_cast<const point*>(t[1]),
                                    *reinterpret_cast<const point*>(t[2]), p)) {
        // If so, mark triangle to be removed and add all edges to the polygon.
        // Count the insertions for each edge to later be able
        // to decide if its a boundary edge.
        bad_triangles.push_back(i);
        ++polygon[{t[0], t[1]}];
        ++polygon[{t[1], t[2]}];
        ++polygon[{t[2], t[0]}];
      }
    }

    // Generate a new triangle connected to the new point for
    // every boundary edge in the polygon.
    size_t bad_id = 0;
    auto edge_it = polygon.begin();
    // There are always more triangles to be inserted than removed.
    for (; bad_id < bad_triangles.size(); ++bad_id, ++edge_it) {
      while (edge_it->second != 1) ++edge_it;
      auto& [e, i] = *edge_it;
      triangles[bad_triangles[bad_id]] =
          triangle{e[0], e[1], reinterpret_cast<size_t>(&p)};
    }
    // Push back the additional triangles.
    for (; edge_it != polygon.end(); ++edge_it) {
      auto& [e, i] = *edge_it;
      if (i != 1) continue;
      triangles.push_back(triangle{e[0], e[1], reinterpret_cast<size_t>(&p)});
    }
  }

  // Compute the result vector of triangles by removing references
  // to the super triangle.
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

// This triangulation precomputes structures for the circumcircle intersection
// routine for every triangle and therefore speeds up the process.
// On the other hand, more memory is needed.
std::vector<triangle> triangulation(std::vector<point>& points) {
  // Construct regular super triangle which contains all given points.
  const auto bounds = bounding_triangle(bounding_circle(bounding_box(points)));
  std::vector<triangle> triangles{
      {reinterpret_cast<size_t>(&bounds[0]),  //
       reinterpret_cast<size_t>(&bounds[1]),  //
       reinterpret_cast<size_t>(&bounds[2])},
  };
  // We already know an upper bound of triangles that will be generated.
  triangles.reserve(2 * points.size() + 3);

  // Precompute circumcircle intersection for super triangle.
  std::vector<std::array<float, 3>> cache{circumcircle_intersection_cache(
      *reinterpret_cast<const point*>(triangles[0][0]),
      *reinterpret_cast<const point*>(triangles[0][1]),
      *reinterpret_cast<const point*>(triangles[0][2]))};
  cache.reserve(2 * points.size() + 3);

  // Initialize structures for the intersection polygon.
  std::map<edge, int> polygon{};
  // std::unordered_map<edge, int, edge::hash> polygon(32);
  // std::set<edge> polygon{};
  // std::unordered_set<edge, edge::hash> polygon{};
  // std::vector<size_t> bad_triangles(32);
  std::vector<size_t> bad_triangles(32);

  // Incrementally add every point.
  for (const auto& p : points) {
    // Clear old polygon.
    polygon.clear();
    bad_triangles.clear();

    // Ask for circumcircle intersection with every triangle.
    for (size_t i = 0; i < triangles.size(); ++i) {
      const auto& t = triangles[i];
      if (circumcircle_intersection(*reinterpret_cast<const point*>(t[0]),
                                    cache[i], p)) {
        // If so, mark triangle to be removed and add all edges to the polygon.
        // Removing edges from the polygon that are no boundary edges
        // does not improve run-time.
        // So we stick to computing how often they were inserted.
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

    // Generate a new triangle connected to the new point for
    // every boundary edge in the polygon.
    size_t bad_id = 0;
    auto edge_it = polygon.begin();
    // There are more new triangles to insert than bad triangles.
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
    // Push back the additional triangles.
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
  // not referencing points of the bounding triangle.
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