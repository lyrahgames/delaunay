#pragma once
#include <algorithm>
#include <array>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lyrahgames::delaunay {

namespace detail {

template <typename F, typename... Args,
          typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type is_valid_implementation(void*);

template <typename F, typename... Args>
std::false_type is_valid_implementation(...);

constexpr auto is_valid = [](auto f) constexpr {
  return [](auto&&... args) constexpr {
    return decltype(
        is_valid_implementation<decltype(f), decltype(args)&&...>(nullptr)){};
  };
};

constexpr auto has_public_xy = is_valid([](auto&& v) -> decltype(v.x * v.y) {});

constexpr auto has_function_xy =
    is_valid([](auto&& v) -> decltype(v.x() * v.y()) {});

constexpr auto has_accesss_operator =
    is_valid([](auto&& v) -> decltype(v[0] * v[1]) {});

}  // namespace detail

struct point {
  float x, y;
};

template <typename Point>
constexpr auto circumcircle_intersection(const Point* a,  //
                                         const Point* b,  //
                                         const Point* c,  //
                                         const Point* p) noexcept
    -> std::enable_if_t<decltype(detail::has_public_xy(*a))::value, bool> {
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

template <typename Point>
constexpr auto circumcircle_intersection(const Point* a,  //
                                         const Point* b,  //
                                         const Point* c,  //
                                         const Point* p) noexcept
    -> std::enable_if_t<!decltype(detail::has_public_xy(*a))::value &&
                            decltype(detail::has_function_xy(*a))::value,
                        bool> {
  const auto axdx = a->x() - p->x();
  const auto aydy = a->y() - p->y();
  const auto bxdx = b->x() - p->x();
  const auto bydy = b->y() - p->y();
  const auto cxdx = c->x() - p->x();
  const auto cydy = c->y() - p->y();
  const auto sqsum_a = axdx * axdx + aydy * aydy;
  const auto sqsum_b = bxdx * bxdx + bydy * bydy;
  const auto sqsum_c = cxdx * cxdx + cydy * cydy;
  const auto det = axdx * (bydy * sqsum_c - cydy * sqsum_b) -
                   aydy * (bxdx * sqsum_c - cxdx * sqsum_b) +
                   sqsum_a * (bxdx * cydy - cxdx * bydy);
  const point edge1{b->x() - a->x(), b->y() - a->y()};
  const point edge2{c->x() - a->x(), c->y() - a->y()};
  const auto d = (edge1.x * edge2.y - edge1.y * edge2.x);
  return (d * det) > 0.0f;
};

template <typename Point>
constexpr auto circumcircle_intersection(const Point* a,  //
                                         const Point* b,  //
                                         const Point* c,  //
                                         const Point* p) noexcept
    -> std::enable_if_t<!decltype(detail::has_public_xy(*a))::value &&
                            !decltype(detail::has_function_xy(*a))::value &&
                            decltype(detail::has_accesss_operator(*a))::value,
                        bool> {
  const auto axdx = (*a)[0] - (*p)[0];
  const auto aydy = (*a)[1] - (*p)[1];
  const auto bxdx = (*b)[0] - (*p)[0];
  const auto bydy = (*b)[1] - (*p)[1];
  const auto cxdx = (*c)[0] - (*p)[0];
  const auto cydy = (*c)[1] - (*p)[1];
  const auto sqsum_a = axdx * axdx + aydy * aydy;
  const auto sqsum_b = bxdx * bxdx + bydy * bydy;
  const auto sqsum_c = cxdx * cxdx + cydy * cydy;
  const auto det = axdx * (bydy * sqsum_c - cydy * sqsum_b) -
                   aydy * (bxdx * sqsum_c - cxdx * sqsum_b) +
                   sqsum_a * (bxdx * cydy - cxdx * bydy);
  const point edge1{(*b)[0] - (*a)[0], (*b)[1] - (*a)[1]};
  const point edge2{(*c)[0] - (*a)[0], (*c)[1] - (*a)[1]};
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

template <typename Point>
std::vector<simplex> triangulation(std::vector<Point>& points) {
  // Construct much larger bounding box for all points.
  const Point bounds[4] = {
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

  // std::unordered_map<facet, int, facet::hash> polytope{};
  std::map<facet, int> polytope{};

  // Incrementally insert every point.
  for (const auto& p : points) {
    // Construct the polytope for a new polytope
    // according to Bowyer and Watson.
    polytope.clear();
    // Test for the circumcircle intersection with every polytope.
    for (auto it = simplices.begin(); it != simplices.end();) {
      auto& t = *it;
      if (circumcircle_intersection(reinterpret_cast<const Point*>(t[0]),
                                    reinterpret_cast<const Point*>(t[1]),
                                    reinterpret_cast<const Point*>(t[2]), &p)) {
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
        static_cast<size_t>(reinterpret_cast<const Point*>(t[0]) - &points[0]);
    const auto b =
        static_cast<size_t>(reinterpret_cast<const Point*>(t[1]) - &points[0]);
    const auto c =
        static_cast<size_t>(reinterpret_cast<const Point*>(t[2]) - &points[0]);

    if ((a < points.size()) && (b < points.size()) && (c < points.size()))
      result.emplace_back(a, b, c);
  }

  return result;
}

namespace experimental {

struct circle {
  point c;
  float r2;
};

constexpr auto circumcircle(const point* a, const point* b,
                            const point* c) noexcept {
  const point edge1{b->x - a->x, b->y - a->y};
  const point edge2{c->x - a->x, c->y - a->y};
  const auto d = 2.0f * (edge1.x * edge2.y - edge1.y * edge2.x);
  const auto inv_d = 1.0f / d;
  const auto sqnorm_edge1 = edge1.x * edge1.x + edge1.y * edge1.y;
  const auto sqnorm_edge2 = edge2.x * edge2.x + edge2.y * edge2.y;
  const point center{inv_d * (edge2.y * sqnorm_edge1 - edge1.y * sqnorm_edge2),
                     inv_d * (edge1.x * sqnorm_edge2 - edge2.x * sqnorm_edge1)};
  return circle{{center.x + a->x, center.y + a->y},
                center.x * center.x + center.y * center.y};
}

constexpr auto intersection(const circle& c, const point* p) noexcept {
  const point r{p->x - c.c.x, p->y - c.c.y};
  return (r.x * r.x + r.y * r.y) <= c.r2;
};

std::vector<simplex> triangulation(std::vector<point>& points) {
  // Construct much larger bounding box for all points.
  const point bounds[4] = {
      {-1.0e3f, -1.0e3f},
      {1.0e3f, -1.0e3f},
      {1.0e3f, 1.0e3f},
      {-1.0e3f, 1.0e3f},
  };
  std::unordered_map<simplex, circle, simplex::hash> simplices{
      std::pair<simplex, circle>{
          {reinterpret_cast<size_t>(&bounds[0]),  //
           reinterpret_cast<size_t>(&bounds[1]),  //
           reinterpret_cast<size_t>(&bounds[2])},
          circumcircle(&bounds[0], &bounds[1], &bounds[2])},
      std::pair<simplex, circle>{
          {reinterpret_cast<size_t>(&bounds[2]),  //
           reinterpret_cast<size_t>(&bounds[3]),  //
           reinterpret_cast<size_t>(&bounds[0])},
          circumcircle(&bounds[2], &bounds[3], &bounds[0])}};

  std::unordered_map<facet, int, facet::hash> polytope{};
  // std::map<facet, int> polytope{};

  // Incrementally insert every point.
  for (const auto& p : points) {
    // Construct the polytope for a new polytope
    // according to Bowyer and Watson.
    polytope.clear();
    // Test for the circumcircle intersection with every polytope.
    for (auto it = simplices.begin(); it != simplices.end();) {
      auto& t = it->first;
      if (intersection(it->second, &p)) {
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
    for (auto& [e, i] : polytope) {
      if (i == 1) {
        const auto c = circumcircle(reinterpret_cast<const point*>(e[0]),
                                    reinterpret_cast<const point*>(e[1]), &p);
        // std::cout << c.c.x << ' ' << c.c.y << ' ' << c.r2 << std::endl;
        simplices.insert({{e[0], e[1], reinterpret_cast<size_t>(&p)}, c});
      }
    }
  }

  // Construct the result vector by adding all simplices
  // not referencing points of the bounding box.
  std::vector<simplex> result{};
  result.reserve(simplices.size());
  for (const auto& [t, _] : simplices) {
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

namespace experimental_3d {

struct point {
  float x, y, z;
};

constexpr point operator+(point x, point y) noexcept {
  return {x.x + y.x, x.y + y.y, x.z + y.z};
}

constexpr point operator-(point x, point y) noexcept {
  return {x.x - y.x, x.y - y.y, x.z - y.z};
}

constexpr point operator*(float x, point v) noexcept {
  return {x * v.x, x * v.y, x * v.z};
}

constexpr auto dot(point x, point y) noexcept {
  return x.x * y.x + x.y * y.y + x.z * y.z;
}

constexpr auto sqnorm(point x) noexcept { return dot(x, x); }

constexpr point cross(point x, point y) noexcept {
  return {x.y * y.z - x.z * y.y,  //
          x.z * y.x - x.x * y.z,  //
          x.x * y.y - x.y * y.x};
}

constexpr point min(point x, point y) noexcept {
  constexpr auto f = [](auto x, auto y) { return (x < y) ? x : y; };
  return {f(x.x, y.x), f(x.y, y.y), f(x.z, y.z)};
}

constexpr point max(point x, point y) noexcept {
  constexpr auto f = [](auto x, auto y) { return (x > y) ? x : y; };
  return {f(x.x, y.x), f(x.y, y.y), f(x.z, y.z)};
}

struct sphere {
  point c;
  float r2;
};

constexpr auto circumsphere(const point& a, const point& b, const point& c,
                            const point& d) noexcept {
  const auto u = b - a;
  const auto v = c - a;
  const auto t = d - a;

  const auto vt_cross = cross(v, t);
  const auto det = 2.0f * dot(u, vt_cross);
  const auto tu_cross = cross(t, u);
  const auto uv_cross = cross(u, v);
  const auto rhs = (1.0f / det) * point{sqnorm(u), sqnorm(v), sqnorm(t)};

  const auto m = rhs.x * vt_cross + rhs.y * tu_cross + rhs.z * uv_cross;

  return sphere{m + a, sqnorm(m)};
}

constexpr auto intersection(const sphere& s, const point& p) noexcept {
  return sqnorm(p - s.c) <= s.r2;
};

struct face : public std::array<size_t, 3> {
  using base_type = std::array<size_t, 3>;

  struct hash {
    constexpr size_t operator()(const face& t) const noexcept {
      return t[0] ^ (t[1] << 3) ^ (t[2] << 5);
    }
  };

  face(size_t a, size_t b, size_t c) : base_type{a, b, c} {
    std::sort(begin(), end());
  }
};

struct tetrahedron : public std::array<size_t, 4> {
  using base_type = std::array<size_t, 4>;

  struct hash {
    constexpr size_t operator()(const tetrahedron& t) const noexcept {
      return t[0] ^ (t[1] << 3) ^ (t[2] << 5) ^ (t[3] << 7);
    }
  };

  tetrahedron(size_t a, size_t b, size_t c, size_t d) : base_type{a, b, c, d} {
    std::sort(begin(), end());
  }
};

struct aabb_t {
  point min{};
  point max{};
};

inline aabb_t aabb(const std::vector<point>& points) noexcept {
  auto p_min = points[0];
  auto p_max = points[0];
  for (size_t i = 1; i < points.size(); ++i) {
    p_min = min(p_min, points[i]);
    p_max = max(p_max, points[i]);
  }
  return {p_min, p_max};
}

constexpr sphere bounding_sphere(const aabb_t& box) noexcept {
  return {0.5 * (box.min + box.max), sqnorm(0.5 * (box.max - box.min))};
}

constexpr std::array<point, 4> bounding_tetrahedron(const sphere& s) noexcept {
  const point a{-0.5, -sqrt(3) / 6, -sqrt(6) / 12};
  const point b{0.5, -sqrt(3) / 6, -sqrt(6) / 12};
  const point c{0, sqrt(3) / 3, -sqrt(6) / 12};
  const point d{0, 0, sqrt(6) / 4};
  const auto k = 2.0 * sqrt(6.0) * sqrt(s.r2);
  return {k * a + s.c, k * b + s.c, k * c + s.c, k * d + s.c};
}

std::vector<tetrahedron> triangulation(const std::vector<point>& points) {
  // Construct regular super tetrahedron which contains all given points.
  const auto box = aabb(points);
  const auto bound_sphere = bounding_sphere(box);
  const auto bounds =
      bounding_tetrahedron({bound_sphere.c, 100 * bound_sphere.r2});
  std::unordered_map<tetrahedron, sphere, tetrahedron::hash> simplices{
      std::pair<tetrahedron, sphere>{
          {reinterpret_cast<size_t>(&bounds[0]),  //
           reinterpret_cast<size_t>(&bounds[1]),  //
           reinterpret_cast<size_t>(&bounds[2]),  //
           reinterpret_cast<size_t>(&bounds[3])},
          circumsphere(bounds[0], bounds[1], bounds[2], bounds[3])}};

  // Construct much larger bounding box for all points.
  // constexpr float big_num = 1.0e3f;
  // const point bounds[8] = {
  //     {-big_num, -big_num, -big_num}, {+big_num, -big_num, -big_num},
  //     {+big_num, +big_num, -big_num}, {-big_num, +big_num, -big_num},
  //     {-big_num, +big_num, +big_num}, {+big_num, +big_num, +big_num},
  //     {+big_num, -big_num, +big_num}, {-big_num, -big_num, +big_num},
  // };
  // std::unordered_map<tetrahedron, sphere, tetrahedron::hash> simplices{
  //     std::pair<tetrahedron, sphere>{
  //         {reinterpret_cast<size_t>(&bounds[0]),  //
  //          reinterpret_cast<size_t>(&bounds[1]),  //
  //          reinterpret_cast<size_t>(&bounds[3]),  //
  //          reinterpret_cast<size_t>(&bounds[7])},
  //         circumsphere(bounds[0], bounds[1], bounds[3], bounds[7])},
  //     std::pair<tetrahedron, sphere>{
  //         {reinterpret_cast<size_t>(&bounds[1]),  //
  //          reinterpret_cast<size_t>(&bounds[2]),  //
  //          reinterpret_cast<size_t>(&bounds[3]),  //
  //          reinterpret_cast<size_t>(&bounds[5])},
  //         circumsphere(bounds[1], bounds[2], bounds[3], bounds[5])},
  //     std::pair<tetrahedron, sphere>{
  //         {reinterpret_cast<size_t>(&bounds[1]),  //
  //          reinterpret_cast<size_t>(&bounds[3]),  //
  //          reinterpret_cast<size_t>(&bounds[5]),  //
  //          reinterpret_cast<size_t>(&bounds[7])},
  //         circumsphere(bounds[1], bounds[3], bounds[5], bounds[7])},
  //     std::pair<tetrahedron, sphere>{
  //         {reinterpret_cast<size_t>(&bounds[1]),  //
  //          reinterpret_cast<size_t>(&bounds[5]),  //
  //          reinterpret_cast<size_t>(&bounds[6]),  //
  //          reinterpret_cast<size_t>(&bounds[7])},
  //         circumsphere(bounds[1], bounds[5], bounds[6], bounds[7])},
  //     std::pair<tetrahedron, sphere>{
  //         {reinterpret_cast<size_t>(&bounds[3]),  //
  //          reinterpret_cast<size_t>(&bounds[4]),  //
  //          reinterpret_cast<size_t>(&bounds[5]),  //
  //          reinterpret_cast<size_t>(&bounds[7])},
  //         circumsphere(bounds[3], bounds[4], bounds[5], bounds[7])},
  // };

  std::unordered_map<face, int, face::hash> polytope{};

  // Incrementally insert every point.
  for (const auto& p : points) {
    // Construct the polytope for a new polytope
    // according to Bowyer and Watson.
    polytope.clear();
    // Test for the circumcircle intersection with every polytope.
    for (auto it = simplices.begin(); it != simplices.end();) {
      auto& t = it->first;
      if (intersection(it->second, p)) {
        // If so, tetrahedron has to be removed and added to the polytope.
        ++polytope[{t[0], t[1], t[2]}];
        ++polytope[{t[1], t[2], t[3]}];
        ++polytope[{t[2], t[3], t[0]}];
        ++polytope[{t[3], t[0], t[1]}];
        it = simplices.erase(it);
      } else {
        ++it;
      }
    }
    // Add new simplices by connecting boundary facets
    // of the polytope with the new point.
    for (auto& [e, i] : polytope) {
      if (i == 1) {
        const auto c = circumsphere(*reinterpret_cast<const point*>(e[0]),
                                    *reinterpret_cast<const point*>(e[1]),
                                    *reinterpret_cast<const point*>(e[2]), p);
        // std::cout << c.c.x << ' ' << c.c.y << ' ' << c.r2 << std::endl;
        simplices.insert({{e[0], e[1], e[2], reinterpret_cast<size_t>(&p)}, c});
      }
    }
  }

  // Construct the result vector by adding all simplices
  // not referencing points of the bounding box.
  std::vector<tetrahedron> result{};
  result.reserve(simplices.size());
  for (const auto& [t, _] : simplices) {
    const auto a =
        static_cast<size_t>(reinterpret_cast<const point*>(t[0]) - &points[0]);
    const auto b =
        static_cast<size_t>(reinterpret_cast<const point*>(t[1]) - &points[0]);
    const auto c =
        static_cast<size_t>(reinterpret_cast<const point*>(t[2]) - &points[0]);
    const auto d =
        static_cast<size_t>(reinterpret_cast<const point*>(t[3]) - &points[0]);

    if ((a < points.size()) && (b < points.size()) && (c < points.size()) &&
        (d < points.size()))
      result.emplace_back(a, b, c, d);
  }

  return result;
}

}  // namespace experimental_3d

}  // namespace lyrahgames::delaunay