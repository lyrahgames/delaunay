#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <lyrahgames/delaunay/delaunay.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;
  using delaunay::point;

  // Initialize oracle for random numbers.
  mt19937 rng{random_device{}()};
  uniform_real_distribution<float> dist{0, 1};
  const auto random = [&] { return dist(rng); };

  // Generate random points.
  vector<point> points(5);
  for (auto& p : points) p = point{random(), random()};

  // Construct Delaunay triangulation.
  const auto elements = delaunay::triangulation(points);

  // Print points.
  cout << setfill('-') << setw(50) << '\n'
       << setfill(' ')  //
       << setw(7) << "Index" << setw(5) << '|' << setw(20) << "Point"
       << '\n'  //
       << setfill('-') << setw(50) << '\n'
       << setfill(' ');
  for (size_t i = 0; i < points.size(); ++i)
    cout << setw(7) << i << setw(5) << '|' << setw(15) << points[i].x << ", "
         << setw(15) << points[i].y << '\n';
  cout << setfill('-') << setw(50) << '\n' << setfill(' ');
  // Print elements.
  cout << setw(7) << "Index" << setw(5) << '|' << setw(20) << "Triangle"
       << '\n'  //
       << setfill('-') << setw(50) << '\n'
       << setfill(' ');
  for (size_t i = 0; i < elements.size(); ++i)
    cout << setw(7) << i << setw(5) << '|' << setw(10) << elements[i][0]
         << setw(10) << elements[i][1] << setw(10) << elements[i][2] << '\n';
  cout << setfill('-') << setw(50) << '\n' << setfill(' ') << flush;
}