#pragma once
#include <array>
#include <vector>
//
#include <lyrahgames/delaunay/geometry.hpp>

namespace lyrahgames::delaunay::guibas_stolfi {

struct alignas(16) edge {
  uintptr_t next{};
  uintptr_t data{};
};

struct alignas(64) quad_edge {
  static constexpr uintptr_t type_mask = 4 * sizeof(edge) - 1;
  static constexpr uintptr_t base_mask = ~type_mask;

  edge& operator[](int index) { return data[index]; }
  const edge& operator[](int index) const { return data[index]; }

  edge data[4]{};
};

static_assert(sizeof(edge) == 16);
static_assert(alignof(edge) == sizeof(edge));
static_assert(sizeof(quad_edge) == 4 * sizeof(edge));
static_assert(alignof(quad_edge) == sizeof(quad_edge));

// Elementary Edge Operations based on pointers
inline auto rotation(edge* e, intptr_t n = 1) noexcept {
  const auto x = reinterpret_cast<uintptr_t>(e);
  const auto y = reinterpret_cast<uintptr_t>(e + n);
  return reinterpret_cast<edge*>((x & quad_edge::base_mask) |
                                 (y & quad_edge::type_mask));
}
inline auto& rotation(edge& e, intptr_t n = 1) noexcept {
  return *rotation(&e, n);
}
inline auto next(edge* e) noexcept { return reinterpret_cast<edge*>(e->next); }
inline auto& next(edge& e) noexcept { return *next(&e); }

// Deduced Edge Operations
inline auto symmetric(edge* e) noexcept { return rotation(e, 2); }
inline auto& symmetric(edge& e) noexcept { return *symmetric(&e); }
inline auto prev(edge* e) noexcept { return rotation(next(rotation(e))); }
inline auto& prev(edge& e) noexcept { return *prev(&e); }

// Operators
inline auto& operator<(edge& e, intptr_t n) noexcept { return rotation(e, n); }
inline auto& operator>(edge& e, intptr_t n) noexcept { return rotation(e, -n); }
inline auto& operator+(edge& e) noexcept { return e < 1; }
inline auto& operator-(edge& e) noexcept { return e > 1; }
inline auto& operator~(edge& e) noexcept { return symmetric(e); }
inline auto& operator++(edge& e) noexcept { return next(e); }
inline auto& operator--(edge& e) noexcept { return prev(e); }

}  // namespace lyrahgames::delaunay::guibas_stolfi