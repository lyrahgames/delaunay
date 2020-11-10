#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <SFML/Graphics.hpp>
//
#include <lyrahgames/delaunay/guibas_stolfi.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;
  using delaunay::guibas_stolfi::edge_algebra;
  using delaunay::guibas_stolfi::point;
  using delaunay::guibas_stolfi::quad_edge;
  using delaunay::guibas_stolfi::subdivision;

  cout << "Press space to regenerate points and triangulation.\n\n" << flush;

  // Initialize oracle for random numbers.
  mt19937 rng{random_device{}()};
  uniform_real_distribution<float> dist{-1, 1};
  const auto random = [&] { return dist(rng); };

  // Initialize containers for points and elements.
  vector<point> points{};
  subdivision edges{};
  edge_algebra delaunay_diagram{};

  const auto generate_points_and_triangulate = [&](size_t n) {
    // Generate random points.
    points.resize(n);
    for (auto& p : points) p = point{random(), random()};
    points[0][0] = -10;
    points[0][1] = -10;
    points[1][0] = 10;
    points[1][1] = -10;
    points[2][0] = 0;
    points[2][1] = 20;

    // sort(&points[3], &points[n], [](auto x, auto y) {
    //   return (x[0] < y[0]) || ((x[0] == y[0]) && (x[1] < y[1]));
    // });
    // cout << "Sorted...done\n" << flush;

    // Construct Delaunay triangulation and measure time taken.
    const auto start = chrono::high_resolution_clock::now();

    delaunay_diagram.edges.resize(0);
    delaunay_diagram.edges.reserve(3 * n);
    delaunay_diagram.set_super_triangle(&points[0], &points[1], &points[2]);
    for (size_t i = 3; i < n; ++i) {
      delaunay_diagram.add(&points[i]);
    }

    const auto end = chrono::high_resolution_clock::now();
    const auto time = chrono::duration<float>(end - start).count();
    cout << "Delaunay triangulation took " << time << " s for " << n
         << " points.\n"
         << "Edge Count = " << delaunay_diagram.edges.size() << '\n'
         << flush;
  };

  constexpr size_t samples = 100000;
  // constexpr size_t samples = 1000000;
  generate_points_and_triangulate(samples);

  // Initialize viewport parameters.
  size_t width = 500;
  size_t height = 500;
  float origin_x = 0;
  float origin_y = 0;
  float fov_y = 2.01f;
  float fov_x = fov_y * width / height;
  const auto projection = [&origin_x, &origin_y, &fov_y, &width, &height](
                              float x, float y) {
    const auto scale = height / fov_y;
    return sf::Vector2f((x - origin_x) * scale + width / 2.0f,
                        (y - origin_y) * scale + height / 2.0f);
  };

  // Construct window for rendering.
  sf::RenderWindow window(sf::VideoMode(width, height),
                          "Delaunay Triangulation Basic Example");
  window.setVerticalSyncEnabled(true);
  int update = 2;

  // Define drawing routines.
  // const auto draw_edge = [&](const simplex& t) {
  const auto draw_edge = [&](const auto& e) {
    array<sf::Vertex, 2> vertices;
    vertices[0] = sf::Vertex(projection((*static_cast<point*>(e[0].data))[0],
                                        (*static_cast<point*>(e[0].data))[1]),
                             sf::Color::Black);
    vertices[1] = sf::Vertex(projection((*static_cast<point*>(e[2].data))[0],
                                        (*static_cast<point*>(e[2].data))[1]),
                             sf::Color::Black);
    window.draw(vertices.data(), vertices.size(), sf::LineStrip);
  };

  const auto draw_point = [&](const point& p) {
    constexpr float radius = 2.5f;
    sf::CircleShape shape(radius);
    shape.setFillColor(sf::Color::Black);
    shape.setOrigin(radius, radius);
    shape.setPosition(projection(p[0], p[1]));
    window.draw(shape);
  };

  // Start the game loop.
  while (window.isOpen()) {
    // Handle events.
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;

        case sf::Event::Resized:
          width = event.size.width;
          height = event.size.height;
          window.setView(sf::View(sf::FloatRect(0, 0, width, height)));
          update = 2;
          break;

        case sf::Event::KeyPressed:
          switch (event.key.code) {
            case sf::Keyboard::Escape:
              window.close();
              break;

            case sf::Keyboard::Space:
              generate_points_and_triangulate(samples);
              update = 2;
              break;

            default:
              break;
          }
          break;

        default:
          break;
      }
    }

    if (update) {
      // Update view.
      fov_x = fov_y * width / height;
      window.clear(sf::Color::White);
      // Render elements and points.
      for (const auto& e : delaunay_diagram.edges) draw_edge(e);
      for (const auto& p : points) draw_point(p);
      --update;
    }

    window.display();
  }
}