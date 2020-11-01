#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lyrahgames::delaunay {

struct point {
  float x, y;
};

struct triangle {
  point vertex[3];
};

struct circle {
  point center;
  float radius;
};

constexpr auto circumcircle(const triangle& t) noexcept {
  using namespace std;

  const point edge1{t.vertex[1].x - t.vertex[0].x,
                    t.vertex[1].y - t.vertex[0].y};
  const point edge2{t.vertex[2].x - t.vertex[0].x,
                    t.vertex[2].y - t.vertex[0].y};
  const auto d = 2.0f * (edge1.x * edge2.y - edge1.y * edge2.x);
  const auto inv_d = 1.0f / d;
  const auto sqnorm_edge1 = edge1.x * edge1.x + edge1.y * edge1.y;
  const auto sqnorm_edge2 = edge2.x * edge2.x + edge2.y * edge2.y;
  const point center{inv_d * (edge2.y * sqnorm_edge1 - edge1.y * sqnorm_edge2),
                     inv_d * (edge1.x * sqnorm_edge2 - edge2.x * sqnorm_edge1)};
  return circle{{center.x + t.vertex[0].x, center.y + t.vertex[0].y},
                sqrt(center.x * center.x + center.y * center.y)};
};

constexpr auto intersection(const triangle& t, const point& p) noexcept {
  const point tp{p.x - t.vertex[0].x, p.y - t.vertex[0].y};
  const point edge1{t.vertex[1].x - t.vertex[0].x,
                    t.vertex[1].y - t.vertex[0].y};
  const point edge2{t.vertex[2].x - t.vertex[0].x,
                    t.vertex[2].y - t.vertex[0].y};
  const auto d = (edge1.x * edge2.y - edge1.y * edge2.x);
  const auto inv_d = 1.0f / d;
  const auto u = inv_d * (edge2.y * tp.x - edge2.x * tp.y);
  const auto v = inv_d * (edge1.x * tp.y - edge1.y * tp.x);
  return ((u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f));
};

constexpr auto circumcircle_intersection(const triangle& t,
                                         const point& p) noexcept {
  const auto axdx = t.vertex[0].x - p.x;
  const auto aydy = t.vertex[0].y - p.y;
  const auto bxdx = t.vertex[1].x - p.x;
  const auto bydy = t.vertex[1].y - p.y;
  const auto cxdx = t.vertex[2].x - p.x;
  const auto cydy = t.vertex[2].y - p.y;
  const auto sqsum_a = axdx * axdx + aydy * aydy;
  const auto sqsum_b = bxdx * bxdx + bydy * bydy;
  const auto sqsum_c = cxdx * cxdx + cydy * cydy;
  const auto det = axdx * (bydy * sqsum_c - cydy * sqsum_b) -
                   aydy * (bxdx * sqsum_c - cxdx * sqsum_b) +
                   sqsum_a * (bxdx * cydy - cxdx * bydy);
  // return det > 0.0f;
  const point edge1{t.vertex[1].x - t.vertex[0].x,
                    t.vertex[1].y - t.vertex[0].y};
  const point edge2{t.vertex[2].x - t.vertex[0].x,
                    t.vertex[2].y - t.vertex[0].y};
  const auto d = (edge1.x * edge2.y - edge1.y * edge2.x);
  return (d * det) > 0.0f;
};

struct triangulation {
  struct edge {
    struct hash {
      constexpr size_t operator()(const edge& e) const noexcept {
        return e.pid[0] ^ (e.pid[1] << 1);
      }
    };

    edge(size_t pid1, size_t pid2)
        : pid{std::min(pid1, pid2), std::max(pid1, pid2)} {}

    size_t pid[2];
  };

  struct triangle {
    struct hash {
      constexpr size_t operator()(const triangle& t) const noexcept {
        return t.pid[0] ^ (t.pid[1] << 1) ^ (t.pid[2] << 2);
      }
    };

    triangle(size_t pid0, size_t pid1, size_t pid2) : pid{pid0, pid1, pid2} {
      if (pid[0] > pid[1]) std::swap(pid[0], pid[1]);
      if (pid[1] > pid[2]) std::swap(pid[1], pid[2]);
      if (pid[0] > pid[1]) std::swap(pid[0], pid[1]);

      // Make sure clockwise and counterclockwise orders are preserved.
      // Move minimal index to start and cyclically interchange others.
      // if (pid[0] < pid[1]) {
      //   if (pid[2] < pid[0]) {
      //     pid[0] = pid2;
      //     pid[1] = pid0;
      //     pid[2] = pid1;
      //   }
      // } else {
      //   if (pid[1] < pid[2]) {
      //     pid[0] = pid1;
      //     pid[1] = pid2;
      //     pid[2] = pid0;
      //   } else {
      //     pid[0] = pid2;
      //     pid[1] = pid0;
      //     pid[2] = pid1;
      //   }
      // }
    }

    size_t pid[3];
  };

  void add(const point& p);

  std::vector<uint32_t> triangle_data(std::vector<point>& data) {
    for (const auto& p : data) add(p);
    std::vector<uint32_t> result{};
    for (auto it = triangles.begin(); it != triangles.end(); ++it) {
      auto& t = *it;
      const auto mask = (~size_t{0}) << 2;
      if ((t.pid[0] & mask) && (t.pid[1] & mask) && (t.pid[2] & mask)) {
        result.push_back(t.pid[0] - 4);
        result.push_back(t.pid[1] - 4);
        result.push_back(t.pid[2] - 4);
      }
    }
    return result;
  }

  template <typename Vector>
  std::vector<uint32_t> triangle_data(const std::vector<Vector>& data) {
    for (const auto& p : data) add({p.x, p.y});
    std::vector<uint32_t> result{};
    for (auto it = triangles.begin(); it != triangles.end(); ++it) {
      auto& t = *it;
      const auto mask = (~size_t{0}) << 2;
      if ((t.pid[0] & mask) && (t.pid[1] & mask) && (t.pid[2] & mask)) {
        result.push_back(t.pid[0] - 4);
        result.push_back(t.pid[1] - 4);
        result.push_back(t.pid[2] - 4);
      }
    }
    return result;
  }

  std::vector<point> points{
      {-1.0e6f, -1.0e6f},
      {1.0e6f, -1.0e6f},
      {1.0e6f, 1.0e6f},
      {-1.0e6f, 1.0e6f},
  };
  std::unordered_set<triangle, triangle::hash> triangles{{0, 1, 2}, {2, 3, 0}};
  std::unordered_map<edge, int, edge::hash> polygon{};
};

constexpr bool operator==(const triangulation::edge& e1,
                          const triangulation::edge& e2) noexcept {
  return (e1.pid[0] == e2.pid[0]) && (e1.pid[1] == e2.pid[1]);
}

constexpr bool operator==(const triangulation::triangle& t1,
                          const triangulation::triangle& t2) noexcept {
  return (t1.pid[0] == t2.pid[0]) && (t1.pid[1] == t2.pid[1]) &&
         (t1.pid[2] == t2.pid[2]);
}

void triangulation::add(const point& p) {
  size_t pid = points.size();
  points.push_back(p);

  polygon.clear();

  for (auto it = triangles.begin(); it != triangles.end();) {
    auto& t = *it;
    if (circumcircle_intersection(
            {{points[t.pid[0]], points[t.pid[1]], points[t.pid[2]]}}, p)) {
      ++polygon[{t.pid[0], t.pid[1]}];
      ++polygon[{t.pid[1], t.pid[2]}];
      ++polygon[{t.pid[2], t.pid[0]}];
      it = triangles.erase(it);
    } else {
      ++it;
    }
  }

  for (auto& [e, i] : polygon) {
    if (i != 1) continue;
    triangles.insert({e.pid[0], e.pid[1], pid});
  }
}

}  // namespace lyrahgames::delaunay