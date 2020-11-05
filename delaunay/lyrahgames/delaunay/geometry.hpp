#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
//
#include <lyrahgames/delaunay/vector.hpp>

namespace lyrahgames::delaunay {

constexpr auto circumcircle_intersection(const float32x2& a,  //
                                         const float32x2& b,  //
                                         const float32x2& c,  //
                                         const float32x2& p) noexcept {
  const auto u = b - a;
  const auto v = c - a;
  const auto r = p - a;

  const auto u2 = sqnorm(u);
  const auto v2 = sqnorm(v);
  const auto r2 = sqnorm(r);

  const auto orientation = u[0] * v[1] - u[1] * v[0];

  const auto determinant = r[0] * (u[1] * v2 - v[1] * u2) -
                           r[1] * (u[0] * v2 - v[0] * u2) + r2 * orientation;

  // return orientation * determinant < 0;
  return (reinterpret_cast<const uint32_t&>(orientation) ^
          reinterpret_cast<const uint32_t&>(determinant)) >>
         31u;
};

constexpr auto circumcircle_intersection_cache(const float32x2& a,  //
                                               const float32x2& b,  //
                                               const float32x2& c) noexcept {
  const auto u = b - a;
  const auto v = c - a;

  const auto u2 = sqnorm(u);
  const auto v2 = sqnorm(v);

  const auto orientation = u[0] * v[1] - u[1] * v[0];
  return std::array<float, 3>{u[1] * v2 - v[1] * u2, u[0] * v2 - v[0] * u2,
                              orientation};
};

constexpr auto circumcircle_intersection(const float32x2& a,
                                         const std::array<float, 3>& cache,
                                         const float32x2& p) noexcept {
  const auto r = p - a;
  const auto r2 = sqnorm(r);
  const auto [x, y, orientation] = cache;
  const auto determinant = r[0] * x - r[1] * y + r2 * orientation;
  // return orientation * determinant < 0;
  return (reinterpret_cast<const uint32_t&>(orientation) ^
          reinterpret_cast<const uint32_t&>(determinant)) >>
         31u;
};

struct aabb {
  float32x2 min{};
  float32x2 max{};
};

struct circle {
  float32x2 center{};
  float radius{};
};

inline auto bounding_box(const std::vector<float32x2>& points) noexcept {
  if (points.size() < 1) return aabb{};
  aabb box{points[0], points[0]};
  for (size_t i = 1; i < points.size(); ++i) {
    box.min = min(box.min, points[i]);
    box.max = max(box.max, points[i]);
  }
  return box;
}

constexpr auto bounding_circle(const aabb& box) noexcept {
  return circle{0.5f * (box.min + box.max), norm(0.5f * (box.max - box.min))};
}

constexpr auto bounding_triangle(const circle& s) noexcept {
  using std::sqrt;

  const float32x2 a{-0.5, -sqrt(3) / 6};
  const float32x2 b{0.5, -sqrt(3) / 6};
  const float32x2 c{0, sqrt(3) / 3};

  const auto k = 2 * sqrt(3.0f) * s.radius * 10;
  return std::array<float32x2, 3>{
      k * a + s.center,  //
      k * b + s.center,  //
      k * c + s.center,  //
  };
}

}  // namespace lyrahgames::delaunay