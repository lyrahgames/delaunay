#include <random>
#include <type_traits>
#include <vector>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/delaunay/vector.hpp>

using doctest::Approx;
using namespace lyrahgames;
using delaunay::vector;
using float32x2 = vector<float, 2>;
using float64x2 = vector<double, 2>;
using float32x3 = vector<float, 3>;
using float64x3 = vector<double, 3>;

TEST_CASE("") {
  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> dist{-1, 1};
  const auto random = [&] { return dist(rng); };

  SUBCASE("Initialization and access for two-dimensional vectors.") {
    const auto x = random();
    const auto y = random();

    float32x2 v{x, y};

    CHECK(v[0] == x);
    CHECK(v[1] == y);

    CHECK(v(0) == x);
    CHECK(v(1) == y);

    SUBCASE("Unary elementwise function application.") {
      const auto a = random();
      const auto b = random();

      const auto u = elementwise(v, [a, b](auto x) { return a * x + b; });

      static_assert(
          std::is_same_v<decltype(u)::value_type, decltype(v)::value_type>);
      static_assert(size(decltype(u){}) == size(decltype(v){}));

      CHECK(Approx(u[0]) == a * x + b);
      CHECK(Approx(u[1]) == a * y + b);

      SUBCASE("Binary elementwise function application.") {
        const auto w =
            elementwise(v, u, [](auto x, auto y) { return x * x + y * y; });

        static_assert(
            std::is_same_v<decltype(w)::value_type, decltype(v)::value_type>);
        static_assert(size(decltype(w){}) == size(decltype(v){}));

        CHECK(Approx(w[0]) == v[0] * v[0] + u[0] * u[0]);
        CHECK(Approx(w[1]) == v[1] * v[1] + u[1] * u[1]);
      }

      SUBCASE("Binary elementwise function application with type change.") {
        const auto w = elementwise(v, u, [](auto x, auto y) { return x <= y; });

        static_assert(std::is_same_v<decltype(w)::value_type, bool>);
        static_assert(size(decltype(w){}) == size(decltype(v){}));

        CHECK(Approx(w[0]) == (v[0] <= u[0]));
        CHECK(Approx(w[1]) == (v[1] <= u[1]));
      }

      SUBCASE("Reduction") {
        const auto w = reduction(
            v, u,  //
            [](auto x, auto y) { return (x < y) ? x : y; },
            [](auto r, auto x) { return (r < x) ? x : r; });

        static_assert(
            std::is_same_v<decltype(w), const decltype(v)::value_type>);

        CHECK(Approx(w) ==
              std::max(std::min(v[0], u[0]), std::min(v[1], u[1])));
      }

      SUBCASE("Addition") {
        const auto w = v + u;

        CHECK(Approx(w[0]) == v[0] + u[0]);
        CHECK(Approx(w[1]) == v[1] + u[1]);
      }

      SUBCASE("Unary Minus") {
        const auto w = +v;

        CHECK(Approx(w[0]) == +v[0]);
        CHECK(Approx(w[1]) == +v[1]);
      }

      SUBCASE("Subtraction") {
        const auto w = v - u;

        CHECK(Approx(w[0]) == v[0] - u[0]);
        CHECK(Approx(w[1]) == v[1] - u[1]);
      }

      SUBCASE("Unary Minus") {
        const auto w = -v;

        CHECK(Approx(w[0]) == -v[0]);
        CHECK(Approx(w[1]) == -v[1]);
      }

      SUBCASE("Left-Handed Multiplication") {
        const auto w = a * v;

        CHECK(Approx(w[0]) == a * v[0]);
        CHECK(Approx(w[1]) == a * v[1]);
      }

      SUBCASE("Right-Handed Multiplication") {
        const auto w = v * a;

        CHECK(Approx(w[0]) == v[0] * a);
        CHECK(Approx(w[1]) == v[1] * a);
      }

      SUBCASE("Multiplication") {
        const auto w = v * u;

        CHECK(Approx(w[0]) == v[0] * u[0]);
        CHECK(Approx(w[1]) == v[1] * u[1]);
      }

      SUBCASE("Left-Handed Division") {
        const auto w = a / v;

        CHECK(Approx(w[0]) == a / v[0]);
        CHECK(Approx(w[1]) == a / v[1]);
      }

      SUBCASE("Right-Handed Division") {
        const auto w = v / a;

        CHECK(Approx(w[0]) == v[0] / a);
        CHECK(Approx(w[1]) == v[1] / a);
      }

      SUBCASE("Division") {
        const auto w = v / u;

        CHECK(Approx(w[0]) == v[0] / u[0]);
        CHECK(Approx(w[1]) == v[1] / u[1]);
      }

      SUBCASE("Dot Product") {
        const auto w = dot(v, u);
        CHECK(Approx(w) == v[0] * u[0] + v[1] * u[1]);
      }

      SUBCASE("Squared Norm") {
        const auto w = sqnorm(v);
        CHECK(Approx(w) == v[0] * v[0] + v[1] * v[1]);
      }

      SUBCASE("Minimum") {
        const auto w = min(v, u);
        CHECK(Approx(w[0]) == std::min(v[0], u[0]));
        CHECK(Approx(w[1]) == std::min(v[1], u[1]));
      }

      SUBCASE("Maximum") {
        const auto w = max(v, u);
        CHECK(Approx(w[0]) == std::max(v[0], u[0]));
        CHECK(Approx(w[1]) == std::max(v[1], u[1]));
      }
    }
  }

  SUBCASE("Initialization and access for two-dimensional constant vectors.") {
    const auto x = random();
    const auto y = random();

    const float32x2 v{x, y};

    CHECK(v[0] == x);
    CHECK(v[1] == y);

    CHECK(v(0) == x);
    CHECK(v(1) == y);
  }

  SUBCASE("Initialization and access for two-dimensional vectors.") {
    const auto x = random();
    const auto y = random();
    const auto z = random();

    float32x3 v{x, y, z};

    CHECK(v[0] == x);
    CHECK(v[1] == y);
    CHECK(v[2] == z);

    CHECK(v(0) == x);
    CHECK(v(1) == y);
    CHECK(v(2) == z);
  }
}