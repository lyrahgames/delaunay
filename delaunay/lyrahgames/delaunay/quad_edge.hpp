#pragma once
#include <vector>

namespace lyrahgames::delaunay {

struct quad_edge_algebra {
  struct alignas(2 * sizeof(size_t)) edge {
    size_t next{};
    size_t data{};
  };

  static_assert(sizeof(edge) == 2 * sizeof(size_t));
  static_assert(alignof(edge) == sizeof(edge));

  static constexpr size_t type_mask = 0b11;
  static constexpr size_t base_mask = ~type_mask;
  static_assert(sizeof(size_t) == sizeof(ptrdiff_t));

  constexpr size_t rotation(size_t eid, ptrdiff_t n = 1) noexcept {
    const auto rid = static_cast<size_t>(eid + n);
    return (eid & base_mask) | (rid & type_mask);
  }
  constexpr size_t symmetric(size_t eid) noexcept { return rotation(eid, 2); }
  constexpr size_t next(size_t eid) noexcept { return edges[eid].next; }
  constexpr size_t previous(size_t eid) noexcept {
    return rotation(next(rotation(eid)));
  }
  constexpr size_t& origin(size_t eid) noexcept { return edges[eid].data; }
  constexpr size_t& destination(size_t eid) noexcept {
    return origin(symmetric(eid));
  }
  constexpr size_t& left(size_t eid) noexcept {
    return origin(rotation(eid, -1));
  }
  constexpr size_t& right(size_t eid) noexcept {
    return origin(rotation(eid, 1));
  }

  constexpr size_t rotl(size_t eid) noexcept { return rotation(eid, 1); }
  constexpr size_t rotr(size_t eid) noexcept { return rotation(eid, -1); }
  constexpr size_t sym(size_t eid) noexcept { return symmetric(eid); }
  constexpr size_t onext(size_t eid) noexcept { return next(eid); }
  constexpr size_t oprev(size_t eid) noexcept { return rotl(onext(rotl(eid))); }
  constexpr size_t dnext(size_t eid) noexcept { return sym(onext(sym(eid))); }
  constexpr size_t dprev(size_t eid) noexcept { return rotr(onext(rotr(eid))); }
  constexpr size_t lnext(size_t eid) noexcept { return rotl(onext(rotr(eid))); }
  constexpr size_t lprev(size_t eid) noexcept { return sym(onext(eid)); }
  constexpr size_t rnext(size_t eid) noexcept { return rotr(onext(rotl(eid))); }
  constexpr size_t rprev(size_t eid) noexcept { return onext(sym(eid)); }
  constexpr size_t& odata(size_t eid) noexcept { return origin(eid); }
  constexpr size_t& ddata(size_t eid) noexcept { return destination(eid); }
  constexpr size_t& ldata(size_t eid) noexcept { return left(eid); }
  constexpr size_t& rdata(size_t eid) noexcept { return right(eid); }

  size_t new_edge() {
    const auto eid = edges.size();
    edges.resize(eid + 4);
    edges[eid + 0].next = eid + 0;
    edges[eid + 1].next = eid + 3;
    edges[eid + 2].next = eid + 2;
    edges[eid + 3].next = eid + 1;
    return eid;
  }

  void splice(size_t a, size_t b) noexcept {
    const auto alpha = rotl(onext(a));
    const auto beta = rotl(onext(b));
    const auto t1 = onext(b);
    const auto t2 = onext(a);
    const auto t3 = onext(beta);
    const auto t4 = onext(alpha);
    edges[a].next = t1;
    edges[b].next = t2;
    edges[alpha].next = t3;
    edges[beta].next = t4;
  }

  size_t connection(size_t a, size_t b) noexcept {
    auto e = new_edge();
    odata(e) = ddata(a);
    ddata(e) = odata(b);
    splice(e, lnext(a));
    splice(sym(e), b);
    return e;
  }

  void remove(size_t e) noexcept {
    splice(e, oprev(e));
    splice(sym(e), oprev(sym(e)));
  }

  void swap(size_t e) noexcept {
    auto a = previous(e);
    auto b = previous(symmetric(e));
    splice(e, a);
    splice(symmetric(e), b);
    splice(e, rotation(next(rotation(a, -1))));
    splice(symmetric(e), rotation(next(rotation(b, -1))));
    origin(e) = destination(a);
    destination(e) = destination(b);
  }

  std::vector<edge> edges;
};

}  // namespace lyrahgames::delaunay