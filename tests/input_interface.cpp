#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <doctest/doctest.h>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>
//
#include <Eigen/Dense>
//
#include <SFML/Graphics.hpp>
//
#include <lyrahgames/delaunay/delaunay.hpp>

struct custom_point {
  float& operator[](size_t i) { return v[i]; }
  const float& operator[](size_t i) const { return v[i]; }
  float v[2];
};

TEST_CASE("Vector types from different libraries can be used as input.") {
  using namespace std;
  using namespace lyrahgames;
  using delaunay::point;

  // Initialize oracle for random numbers.
  mt19937 rng{random_device{}()};
  uniform_real_distribution<float> dist{0, 1};
  const auto random = [&] { return dist(rng); };

  const size_t samples = 1000;

  for (size_t i = 0; i < 100; ++i) {
    // Generate random points for both types.
    vector<point> points(samples);
    vector<glm::vec2> glm_points(samples);
    vector<Eigen::Vector2f> eigen_points(samples);
    vector<custom_point> custom_points(samples);
    vector<sf::Vector2f> sfml_points(samples);
    for (size_t i = 0; i < samples; ++i) {
      const auto x = random();
      const auto y = random();
      points[i] = point{x, y};
      glm_points[i] = glm::vec2{x, y};
      eigen_points[i] = Eigen::Vector2f{x, y};
      custom_points[i] = custom_point{x, y};
      sfml_points[i] = sf::Vector2f{x, y};
    }

    // Construct Delaunay triangulation for both point types.
    auto elements = delaunay::triangulation(points);
    sort(begin(elements), end(elements));
    auto glm_elements = delaunay::triangulation(glm_points);
    sort(begin(glm_elements), end(glm_elements));
    auto eigen_elements = delaunay::triangulation(eigen_points);
    sort(begin(eigen_elements), end(eigen_elements));
    auto custom_elements = delaunay::triangulation(custom_points);
    sort(begin(custom_elements), end(custom_elements));
    auto sfml_elements = delaunay::triangulation(sfml_points);
    sort(begin(sfml_elements), end(sfml_elements));

    REQUIRE(elements == glm_elements);
    REQUIRE(elements == eigen_elements);
    REQUIRE(elements == custom_elements);
    REQUIRE(elements == sfml_elements);
  }
}