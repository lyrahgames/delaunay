#include <random>
#include <vector>
//
#include <perfevent/perfevent.hpp>
//
#include <lyrahgames/delaunay/delaunay.hpp>

using namespace std;
using namespace lyrahgames;
using delaunay::point;
using delaunay::simplex;

struct benchmark {
  benchmark& run(size_t n) noexcept {
    points.resize(n);
    for (auto& p : points) p = point{dist(rng), dist(rng)};
    elements.clear();
    {
      params.setParam("points", n);
      PerfEventBlock e(n, params, header);
      elements = delaunay::triangulation(points);
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
  vector<simplex> elements{};
};

int main() {
  benchmark{}  //
      .run(100)
      .run(1000)
      .run(10000)
      .run(20000)
      // .run(30000)
      ;
}
