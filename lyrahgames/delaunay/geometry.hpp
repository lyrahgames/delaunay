#pragma once
#include <cstdint>
//
#include <glm/glm.hpp>

namespace lyrahgames::delaunay {

using point = glm::vec2;

inline auto counterclockwise(const point& a,
                             const point& b,
                             const point& c) noexcept {
  const auto u = b - a;
  const auto v = c - a;
  return (u[0] * v[1] - u[1] * v[0]) > 0;
}

inline auto clockwise(const point& a, const point& b, const point& c) noexcept {
  const auto u = b - a;
  const auto v = c - a;
  return (u[0] * v[1] - u[1] * v[0]) < 0;
}

inline auto circumcircle_intersection(const point& a,  //
                                      const point& b,  //
                                      const point& c,  //
                                      const point& p) noexcept {
  const auto u = b - a;
  const auto v = c - a;
  const auto r = p - a;

  const auto u2 = dot(u, u);
  const auto v2 = dot(v, v);
  const auto r2 = dot(r, r);

  const auto orientation = u[0] * v[1] - u[1] * v[0];

  const auto determinant = r[0] * (u[1] * v2 - v[1] * u2) -
                           r[1] * (u[0] * v2 - v[0] * u2) + r2 * orientation;

  // return orientation * determinant < 0;
  return (reinterpret_cast<const uint32_t&>(orientation) ^
          reinterpret_cast<const uint32_t&>(determinant)) >>
         31u;
};

}  // namespace lyrahgames::delaunay