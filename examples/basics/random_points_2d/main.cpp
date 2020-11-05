#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <SFML/Graphics.hpp>
//
#include <lyrahgames/delaunay/bowyer_watson.hpp>
#include <lyrahgames/delaunay/delaunay.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;
  // using delaunay::point;
  // using delaunay::simplex;
  using delaunay::bowyer_watson::point;
  using delaunay::bowyer_watson::triangle;

  cout << "Press space to regenerate points and triangulation.\n\n" << flush;

  // Initialize oracle for random numbers.
  mt19937 rng{random_device{}()};
  uniform_real_distribution<float> dist{-1, 1};
  const auto random = [&] { return dist(rng); };

  // Initialize containers for points and elements.
  vector<point> points{};
  // vector<simplex> elements{};
  vector<triangle> elements{};

  const auto generate_points_and_triangulate = [&](size_t n) {
    // Generate random points.
    points.resize(n);
    for (auto& p : points) p = point{random(), random()};

    // Construct Delaunay triangulation and measure time taken.
    const auto start = chrono::high_resolution_clock::now();
    // elements = delaunay::triangulation(points);
    elements = delaunay::bowyer_watson::triangulation(points);
    const auto end = chrono::high_resolution_clock::now();
    const auto time = chrono::duration<float>(end - start).count();
    cout << "Delaunay triangulation took " << time << " s for " << n
         << " points.\n"
         << flush;
  };

  constexpr size_t samples = 10000;
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
  // const auto draw_triangle = [&](const simplex& t) {
  const auto draw_triangle = [&](const triangle& t) {
    array<sf::Vertex, 4> vertices;
    vertices[0] = sf::Vertex(projection(points[t[0]].x, points[t[0]].y),
                             sf::Color::Black);
    vertices[1] = sf::Vertex(projection(points[t[1]].x, points[t[1]].y),
                             sf::Color::Black);
    vertices[2] = sf::Vertex(projection(points[t[2]].x, points[t[2]].y),
                             sf::Color::Black);
    vertices[3] = sf::Vertex(projection(points[t[0]].x, points[t[0]].y),
                             sf::Color::Black);
    window.draw(vertices.data(), vertices.size(), sf::LineStrip);
  };

  const auto draw_point = [&](const point& p) {
    constexpr float radius = 2.5f;
    sf::CircleShape shape(radius);
    shape.setFillColor(sf::Color::Black);
    shape.setOrigin(radius, radius);
    shape.setPosition(projection(p.x, p.y));
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
      for (const auto& e : elements) draw_triangle(e);
      for (const auto& p : points) draw_point(p);
      --update;
    }

    window.display();
  }
}