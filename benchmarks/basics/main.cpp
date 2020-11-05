#include <random>
#include <vector>
//
#include <perfevent/perfevent.hpp>
//
// #include <lyrahgames/delaunay/delaunay.hpp>
#include <lyrahgames/delaunay/bowyer_watson.hpp>

using namespace std;
using namespace lyrahgames;
using delaunay::bowyer_watson::point;
using delaunay::bowyer_watson::triangle;

struct benchmark {
  benchmark& run(size_t n) noexcept {
    points.resize(n);
    for (auto& p : points) p = point{dist(rng), dist(rng)};
    triangles.clear();
    {
      params.setParam("          name", "default");
      params.setParam("points", n);
      PerfEventBlock e(n, params, header);
      triangles = delaunay::bowyer_watson::triangulation(points);
    }
    header = false;
    return *this;
  }

  benchmark& run_experimental(size_t n) noexcept {
    points.resize(n);
    for (auto& p : points) p = point{dist(rng), dist(rng)};
    triangles.clear();
    {
      params.setParam("          name", "experimental");
      params.setParam("points", n);
      PerfEventBlock e(n, params, header);
      triangles = delaunay::bowyer_watson::experimental::triangulation(points);
    }
    header = false;
    return *this;
  }

  benchmark& separate() noexcept {
    header = true;
    cout << "\n";
    return *this;
  }

  benchmark& heading(const char* text) {
    cout << text << "\n";
    return *this;
  }

  BenchmarkParameters params{};
  bool header = true;
  mt19937 rng{random_device{}()};
  uniform_real_distribution<float> dist{0, 1};
  vector<point> points{};
  vector<triangle> triangles{};
};

int main() {
  benchmark{}  //
      .run(100)
      .run_experimental(100)
      .run(1000)
      .run_experimental(1000)
      .run(10000)
      .run_experimental(10000)
      .run(20000)
      .run_experimental(20000)
      .run(100000)
      .run_experimental(100000)
      //
      ;
}
