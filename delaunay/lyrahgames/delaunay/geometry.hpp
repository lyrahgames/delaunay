#pragma once
#include <cstdint>
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

}  // namespace lyrahgames::delaunay