#pragma once
#include <array>
#include <vector>
//
#include <lyrahgames/delaunay/geometry.hpp>

namespace lyrahgames::delaunay::guibas_stolfi {

using point = float32x2;

struct alignas(16) edge {
  uintptr_t next{};
  uintptr_t data{};

  struct traversor {
    edge* next;
  };
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

struct subdivision {
  std::vector<quad_edge> edges;

  void make_edge(size_t index, point* o, point* d) {
    auto& e = edges[index];
    e[0].next = reinterpret_cast<uintptr_t>(&e[0]);
    e[0].data = reinterpret_cast<uintptr_t>(o);

    e[1].next = reinterpret_cast<uintptr_t>(&e[3]);
    e[1].data = 0;  // Null Face

    e[2].next = reinterpret_cast<uintptr_t>(&e[2]);
    e[2].data = reinterpret_cast<uintptr_t>(d);

    e[3].next = reinterpret_cast<uintptr_t>(&e[1]);
    e[3].data = 0;  // Null Face
  }

  void splice(edge& a, edge& b) {
    auto& alpha = rotation(next(a));
    auto& beta = rotation(next(b));

    auto& t1 = next(b);
    auto& t2 = next(a);
    auto& t3 = next(beta);
    auto& t4 = next(alpha);

    a.next = reinterpret_cast<uintptr_t>(&t1);
    b.next = reinterpret_cast<uintptr_t>(&t2);
    alpha.next = reinterpret_cast<uintptr_t>(&t3);
    beta.next = reinterpret_cast<uintptr_t>(&t4);
  }
};

struct edge_algebra {
  struct alignas(2 * sizeof(void*)) edge {
    edge* next{};
    void* data{};
  };

  struct alignas(4 * sizeof(edge)) quad_edge {
    edge& operator[](int index) { return data[index]; }
    const edge& operator[](int index) const { return data[index]; }
    edge data[4]{};
  };

  static_assert(sizeof(edge) == 2 * sizeof(void*));
  static_assert(alignof(edge) == sizeof(edge));
  static_assert(sizeof(quad_edge) == 4 * sizeof(edge));
  static_assert(alignof(quad_edge) == sizeof(quad_edge));

  // byte index in quad_edge structure
  static constexpr uintptr_t type_mask = 4 * sizeof(edge) - 1;
  // quad_edge memory position
  static constexpr uintptr_t base_mask = ~type_mask;

  struct traversor {
    edge* pointer{};
    auto operator*() noexcept;
  };

  auto new_edge() noexcept;
  auto new_edge(size_t index) noexcept;
  void splice(edge* a, edge* b) noexcept;
  auto connection(edge* a, edge* b) noexcept;
  void remove(edge* e) noexcept;
  void swap(edge* e) noexcept;
  auto right_of(const point& x, edge* e) noexcept;
  auto left_of(const point& x, edge* e) noexcept;
  auto locate(const point& x) noexcept;
  void add(point* p) noexcept;
  void set_super_triangle(point* a, point* b, point* c) noexcept;

  std::vector<quad_edge> edges;
};

inline auto rotation(edge_algebra::edge* e, intptr_t n = 1) noexcept {
  const auto x = reinterpret_cast<uintptr_t>(e);
  const auto y = reinterpret_cast<uintptr_t>(e + n);
  return reinterpret_cast<edge_algebra::edge*>((x & edge_algebra::base_mask) |
                                               (y & edge_algebra::type_mask));
}
inline auto symmetric(edge_algebra::edge* e) noexcept { return rotation(e, 2); }
inline auto next(edge_algebra::edge* e) noexcept { return e->next; }
inline auto previous(edge_algebra::edge* e) noexcept {
  return rotation(next(rotation(e)));
}
inline auto& origin(edge_algebra::edge* e) noexcept { return e->data; }
inline auto& destination(edge_algebra::edge* e) noexcept {
  return origin(symmetric(e));
}
inline auto& left(edge_algebra::edge* e) noexcept {
  return origin(rotation(e, -1));
}
inline auto& right(edge_algebra::edge* e) noexcept {
  return origin(rotation(e, 1));
}

inline auto operator+(edge_algebra::traversor t) noexcept {
  return edge_algebra::traversor{rotation(t.pointer, 1)};
}
inline auto operator-(edge_algebra::traversor t) noexcept {
  return edge_algebra::traversor{rotation(t.pointer, -1)};
}
inline auto operator~(edge_algebra::traversor t) noexcept {
  return edge_algebra::traversor{symmetric(t.pointer)};
}
inline auto operator++(edge_algebra::traversor t) noexcept {
  return edge_algebra::traversor{next(t.pointer)};
}
inline auto operator--(edge_algebra::traversor t) noexcept {
  return edge_algebra::traversor{previous(t.pointer)};
}
inline auto operator++(edge_algebra::traversor t, int) noexcept {
  return ++(~t);
}
inline auto operator--(edge_algebra::traversor t, int) noexcept {
  return +(++(-t));
}
inline auto edge_algebra::traversor::operator*() noexcept {
  return origin(pointer);
}

inline auto edge_algebra::new_edge(size_t index) noexcept {
  auto& e = edges[index];
  e[0].next = &e[0];
  e[1].next = &e[3];
  e[2].next = &e[2];
  e[3].next = &e[1];
  return &e[0];
}

inline auto edge_algebra::new_edge() noexcept {
  // Assume there is no reallocation of the edges vector.
  const auto index = edges.size();
  edges.push_back({});
  return new_edge(index);
}

inline void edge_algebra::splice(edge* a, edge* b) noexcept {
  auto alpha = rotation(next(a));
  auto beta = rotation(next(b));
  auto t1 = next(b);
  auto t2 = next(a);
  auto t3 = next(beta);
  auto t4 = next(alpha);
  a->next = t1;
  b->next = t2;
  alpha->next = t3;
  beta->next = t4;
}

inline auto edge_algebra::connection(edge* a, edge* b) noexcept {
  auto e = new_edge();
  origin(e) = destination(a);
  destination(e) = origin(b);
  splice(e, rotation(next(rotation(a, -1))));
  splice(symmetric(e), b);
  return e;
}

inline void edge_algebra::remove(edge* e) noexcept {
  splice(e, previous(e));
  splice(symmetric(e), previous(symmetric(e)));
}

inline void edge_algebra::swap(edge* e) noexcept {
  auto a = previous(e);
  auto b = previous(symmetric(e));
  splice(e, a);
  splice(symmetric(e), b);
  splice(e, rotation(next(rotation(a, -1))));
  splice(symmetric(e), rotation(next(rotation(b, -1))));
  origin(e) = destination(a);
  destination(e) = destination(b);
}

inline auto edge_algebra::right_of(const point& x, edge* e) noexcept {
  return counterclockwise(x, *static_cast<point*>(destination(e)),
                          *static_cast<point*>(origin(e)));
}

inline auto edge_algebra::left_of(const point& x, edge* e) noexcept {
  return counterclockwise(x, *static_cast<point*>(origin(e)),
                          *static_cast<point*>(destination(e)));
}

inline auto edge_algebra::locate(const point& x) noexcept {
  auto e = &edges[0][0];
  while (true) {
    if (right_of(x, e))
      e = symmetric(e);
    else if (!right_of(x, next(e)))
      e = next(e);
    else if (!right_of(x, rotation(next(rotation(e, -1)), -1)))
      e = rotation(next(rotation(e, -1)), -1);
    else
      return e;
  }
}

inline void edge_algebra::add(point* p) noexcept {
  auto& x = *p;
  auto e = locate(x);
  auto base = new_edge();
  origin(base) = origin(e);
  destination(base) = p;
  splice(base, e);

  auto first = base;
  do {
    base = connection(e, symmetric(base));
    e = previous(base);
  } while (rotation(next(rotation(e, -1))) != first);

  do {
    auto t = previous(e);
    if (right_of(*static_cast<point*>(destination(t)), e) &&
        circumcircle_intersection(*static_cast<point*>(origin(e)),
                                  *static_cast<point*>(destination(t)),
                                  *static_cast<point*>(destination(e)), x)) {
      swap(e);
      e = previous(e);
    } else if (next(e) == first)
      return;
    else
      e = symmetric(next(next(e)));
  } while (true);
}

inline void edge_algebra::set_super_triangle(point* a, point* b,
                                             point* c) noexcept {
  auto u = new_edge();
  origin(u) = a;
  destination(u) = b;

  auto v = new_edge();
  origin(v) = b;
  destination(v) = c;

  splice(symmetric(u), v);

  auto t = new_edge();
  origin(t) = c;
  destination(t) = a;

  splice(symmetric(v), t);
  splice(symmetric(t), u);
}

}  // namespace lyrahgames::delaunay::guibas_stolfi