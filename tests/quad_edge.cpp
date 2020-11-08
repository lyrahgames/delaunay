#include <array>
#include <vector>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/delaunay/guibas_stolfi.hpp>

using namespace std;
using namespace lyrahgames;
using namespace delaunay::guibas_stolfi;
using delaunay::float32x2;

TEST_CASE("") {
  float32x2 points[] = {{1, 2}, {3, 4}, {5, 6}};
  array<size_t, 3> triangles[] = {{0, 1, 2}};
  quad_edge edges[3];
  edges[0][0].next = reinterpret_cast<uintptr_t>(&(edges[2][2]));
  edges[0][0].data = reinterpret_cast<uintptr_t>(&points[0]);

  edges[0][1].next = reinterpret_cast<uintptr_t>(&(edges[2][1]));
  edges[0][1].data = 0;

  edges[0][2].next = reinterpret_cast<uintptr_t>(&(edges[1][0]));
  edges[0][2].data = reinterpret_cast<uintptr_t>(&points[1]);

  edges[0][3].next = reinterpret_cast<uintptr_t>(&(edges[1][3]));
  edges[0][3].data = reinterpret_cast<uintptr_t>(&triangles[0]);

  edges[1][0].next = reinterpret_cast<uintptr_t>(&(edges[0][2]));
  edges[1][0].data = reinterpret_cast<uintptr_t>(&points[1]);

  edges[1][1].next = reinterpret_cast<uintptr_t>(&(edges[0][1]));
  edges[1][1].data = 0;

  edges[1][2].next = reinterpret_cast<uintptr_t>(&(edges[2][0]));
  edges[1][2].data = reinterpret_cast<uintptr_t>(&points[2]);

  edges[1][3].next = reinterpret_cast<uintptr_t>(&(edges[2][3]));
  edges[1][3].data = reinterpret_cast<uintptr_t>(&triangles[0]);

  edges[2][0].next = reinterpret_cast<uintptr_t>(&(edges[1][2]));
  edges[2][0].data = reinterpret_cast<uintptr_t>(&points[2]);

  edges[2][1].next = reinterpret_cast<uintptr_t>(&(edges[1][1]));
  edges[2][1].data = 0;

  edges[2][2].next = reinterpret_cast<uintptr_t>(&(edges[0][0]));
  edges[2][2].data = reinterpret_cast<uintptr_t>(&points[0]);

  edges[2][3].next = reinterpret_cast<uintptr_t>(&(edges[0][3]));
  edges[2][3].data = reinterpret_cast<uintptr_t>(&triangles[0]);

  const auto first = &edges[0][0];
  const auto second = &edges[1][0];
  const auto third = &edges[2][0];

  REQUIRE(next(next(first)) == first);
  REQUIRE(next(next(second)) == second);
  REQUIRE(next(next(third)) == third);

  REQUIRE(second == rotation(next(rotation(first, -1))));
  REQUIRE(third == rotation(next(rotation(second, -1))));
  REQUIRE(first == rotation(next(rotation(third, -1))));

  REQUIRE(symmetric(third) == next(first));
  REQUIRE(symmetric(first) == next(second));
  REQUIRE(symmetric(second) == next(third));

  REQUIRE(rotation(first) == symmetric(rotation(first, -1)));
  REQUIRE(rotation(second) == symmetric(rotation(second, -1)));
  REQUIRE(rotation(third) == symmetric(rotation(third, -1)));

  REQUIRE(next(rotation(first, -1)) == rotation(second, -1));
  REQUIRE(next(rotation(second, -1)) == rotation(third, -1));
  REQUIRE(next(rotation(third, -1)) == rotation(first, -1));

  REQUIRE(next(first) == prev(first));
  REQUIRE(next(second) == prev(second));
  REQUIRE(next(third) == prev(third));

  REQUIRE(next(next(rotation(first, -1))) == prev(rotation(first, -1)));
  REQUIRE(next(next(rotation(second, -1))) == prev(rotation(second, -1)));
  REQUIRE(next(next(rotation(third, -1))) == prev(rotation(third, -1)));
}