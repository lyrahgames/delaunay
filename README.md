<h1 align="center">
    Lyrahgames' Delaunay Tessellation Package
</h1>

<p align="center">
    Header-Only C++ Library for Generating the N-dimensional Delaunay Triangulation/Tessellation of a Set of Given Points
</p>

<table align="center">
    <tr>
        <td><img src="docs/images/random_points_3d.png" width="300"/></td>
        <td><img src="docs/images/random_points_3d_surface.png" width="300"/></td>
    </tr>
</table>

[Read the report.](docs/report/main.pdf)

## Development Status

<p align="center">
    <img src="https://img.shields.io/github/languages/top/lyrahgames/delaunay.svg?style=for-the-badge">
    <img src="https://img.shields.io/github/languages/code-size/lyrahgames/delaunay.svg?style=for-the-badge">
    <img src="https://img.shields.io/github/repo-size/lyrahgames/delaunay.svg?style=for-the-badge">
    <a href="COPYING.md">
        <img src="https://img.shields.io/github/license/lyrahgames/delaunay.svg?style=for-the-badge&color=blue">
    </a>
</p>

<b>
<table align="center">
    <tr>
        <td>
            master
        </td>
        <td>
            <a href="https://github.com/lyrahgames/delaunay">
                <img src="https://img.shields.io/github/last-commit/lyrahgames/delaunay/master.svg?logo=github&logoColor=white">
            </a>
        </td>    
        <!-- <td>
            <a href="https://circleci.com/gh/lyrahgames/delaunay/tree/master"><img src="https://circleci.com/gh/lyrahgames/delaunay/tree/master.svg?style=svg"></a>
        </td> -->
        <!-- <td>
            <a href="https://codecov.io/gh/lyrahgames/delaunay">
              <img src="https://codecov.io/gh/lyrahgames/delaunay/branch/master/graph/badge.svg" />
            </a>
        </td> -->
        <td>
            <a href="https://ci.cppget.org/?builds=lyrahgames-robin-hood&pv=&tc=*&cf=&mn=&tg=&rs=*">
                <img src="https://img.shields.io/badge/b|2 ci.cppget.org-Click here!-blue">
            </a>
        </td>
    </tr>
    <!-- <tr>
        <td>
            develop
        </td>
        <td>
            <a href="https://github.com/lyrahgames/delaunay/tree/develop">
                <img src="https://img.shields.io/github/last-commit/lyrahgames/delaunay/develop.svg?logo=github&logoColor=white">
            </a>
        </td>    
        <td>
            <a href="https://circleci.com/gh/lyrahgames/delaunay/tree/develop"><img src="https://circleci.com/gh/lyrahgames/delaunay/tree/develop.svg?style=svg"></a>
        </td>
        <td>
            <a href="https://codecov.io/gh/lyrahgames/delaunay">
              <img src="https://codecov.io/gh/lyrahgames/delaunay/branch/develop/graph/badge.svg" />
            </a>
        </td>
    </tr> -->
    <tr>
        <td>
        </td>
    </tr>
    <tr>
        <td>
            Current
        </td>
        <td>
            <a href="https://github.com/lyrahgames/delaunay">
                <img src="https://img.shields.io/github/commit-activity/y/lyrahgames/delaunay.svg?logo=github&logoColor=white">
            </a>
        </td>
        <!-- <td>
            <img src="https://img.shields.io/github/release/lyrahgames/delaunay.svg?logo=github&logoColor=white">
        </td>
        <td>
            <img src="https://img.shields.io/github/release-pre/lyrahgames/delaunay.svg?label=pre-release&logo=github&logoColor=white">
        </td> -->
        <td>
            <img src="https://img.shields.io/github/tag/lyrahgames/delaunay.svg?logo=github&logoColor=white">
        </td>
        <td>
            <img src="https://img.shields.io/github/tag-date/lyrahgames/delaunay.svg?label=latest%20tag&logo=github&logoColor=white">
        </td>
        <!-- <td>
            <a href="https://queue.cppget.org/robin-hood">
                <img src="https://img.shields.io/website/https/queue.cppget.org/robin-hood.svg?down_message=empty&down_color=blue&label=b|2%20queue.cppget.org&up_color=orange&up_message=running">
            </a>
        </td> -->
    </tr>
</table>
</b>

## Requirements
<b>
<table>
    <tr>
        <td>Language Standard:</td>
        <td>C++20</td>
    </tr>
    <tr>
        <td>Compiler:</td>
        <td>
            GCC | Clang | MinGW
        </td>
    </tr>
    <tr>
        <td>Build System:</td>
        <td>
            <a href="https://build2.org/">build2</a>
        </td>
    </tr>
    <tr>
        <td>Operating System:</td>
        <td>
            Linux | Windows | MacOS
        </td>
    </tr>
    <tr>
        <td>Dependencies:</td>
        <td>
            <a href="https://github.com/lyrahgames/xstd">
                lyrahgames-xstd
            </a>
            <br>
            <a href="https://cppget.org/doctest">
                doctest
            </a>
        </td>
    </tr>
</table>
</b>

## Installation
The standard installation process will only install the header-only library.
If you are interested in installing examples, benchmarks, or tests, you have to run these installation commands manually.

    bpkg -d build2-packages cc \
      config.install.root=/usr/local \
      config.install.sudo=sudo

Get the latest package release and build it.

    bpkg build https://github.com/lyrahgames/delaunay.git

Install the built package.

    bpkg install lyrahgames-delaunay

For uninstalling, do the following.

    bpkg uninstall lyrahgames-delaunay

If your package uses an explicit `depends: lyrahgames-delaunay` make sure to initialize this dependency as a system dependency when creating a new configuration.

    bdep init -C @build cc config.cxx=g++ "config.cxx.coptions=-O3" -- "?sys:lyrahgames-delaunay/*"

## Usage with build2
For now, there is no official package release on `cppget.org`.
Therefore add this repository in your `repositories.manifest` file of your package.

    :
    role: prerequisite
    location: https://github.com/lyrahgames/delaunay#master

Add the following entry to the `manifest` file of your package with an optional version dependency.

    depends: lyrahgames-delaunay

Then import the library in your `buildfile` to be able to link against your own executable or library.

    import libs = lyrahgames-delaunay%lib{lyrahgames-delaunay}
    exe{myexe}: {hxx cxx}{**} $libs

## Examples

```c++
#include <random>
#include <vector>
//
#include <lyrahgames/delaunay/bowyer_watson.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;
  using delaunay::bowyer_watson::point;

  // Initialize oracle for random numbers.
  mt19937 rng{random_device{}()};
  uniform_real_distribution<float> dist{0, 1};
  const auto random = [&] { return dist(rng); };

  // Generate random points.
  vector<point> points(5);
  for (auto& p : points) p = point{random(), random()};

  // Construct Delaunay triangulation.
  const auto elements = delaunay::bowyer_watson::triangulation(points);
}
```

![](docs/images/random_points_2d.png)

|3D Triangulation | Surface Triangles |
|---|---|
| ![](docs/images/random_points_3d.png) | ![](docs/images/random_points_3d_surface.png) |

## Tests

## Benchmarks

## API

## Features

## Background
### Circumcircles and Circumspheres
### Bowyer-Watson Algorithm

## Implementation Details

## References

- [Lee and Schachter, *Two Algorithms for Constructing a Delaunay Triangulation*, 1980](http://www.personal.psu.edu/cxc11/AERSP560/DELAUNEY/13_Two_algorithms_Delauney.pdf)
- [Guibas and Stolfi, *Primitives for the Manipulation of General Subdivisions and the Computation of Voronoi Diagrams*, 1985](http://sccg.sk/~samuelcik/dgs/quad_edge.pdf)
- [Dwyer, *A Faster Divide-and-Conquer Algorithm for Constructing Delaunay Triangulations*, 1986](https://link.springer.com/article/10.1007/BF01840356)
- [Cignoni, Montani and Scopigno, *DeWall: A Fast Divide-and-Conquer Delaunay Triangulation Algorithm in E^d*, 1998](https://www.sciencedirect.com/science/article/abs/pii/S0010448597000821)
- [Fuetterling, Lojewski and Pfreundt, *High-Performance Delaunay Triangulation for Many-Core Computers*, 2014](https://diglib.eg.org/bitstream/handle/10.2312/hpg.20141098.097-104/097-104.pdf?sequence=1)
- [Slides to Fuetterling, Lojewski and Pfreundt, *High-Performance Delaunay Triangulation for Many-Core Computers*, 2014](https://www.highperformancegraphics.org/wp-content/uploads/2014/Fuetterling-Delaunay.pdf)
- [Shewchuk, *Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator*](https://people.eecs.berkeley.edu/~jrs/papers/triangle.pdf)
- [Lischinski, *Incremental Delaunay Triangulation*, 1993](http://www.karlchenofhell.org/cppswp/lischinski.pdf)
- [Burnikel, *Delaunay Graphs by Divide and Conquer*, 1998](https://pure.mpg.de/rest/items/item_1819432_4/component/file_2599484/content)
- [Katajainen and Koppinen, *Constructing Delaunay Triangulations by Merging Buckets in Quadtree Order*, 1988](http://hjemmesider.diku.dk/~jyrki/Paper/KK88.pdf)
- [Watson, *Computing the N-Dimensional Delaunay Tessellation with Application to Voronoi Polytopes*, 1981](https://academic.oup.com/comjnl/article/24/2/167/338200)
- [Bowyer, *Computing Dirichlet Tessellations*, 1981](https://academic.oup.com/comjnl/article/24/2/162/338193)

- [CP-Algorithms, *Delaunay Triangulation and Voronoi Diagram*, 2014](https://cp-algorithms.com/geometry/delaunay.html)
- [*Quad Edge Data Structure*](https://www.neolithicsphere.com/geodesica/doc/quad_edge_overview.htm)
- [Shewchuk, *TRIANGLE: Mesh Generation and Delaunay Triangulation*](https://people.sc.fsu.edu/~jburkardt/c_src/triangle/triangle.html)
- [*Delaunay Triangulation in Two and Three Dimensions*](http://www.ae.metu.edu.tr/tuncer/ae546/prj/delaunay/)
- [Peterson, *Computing Constrained Delaunay Triangulations: In the Plane*](http://www.geom.uiuc.edu/~samuelp/del_project.html)
- [GitHub/Samson-Mano/Delaunay-triangulation](https://github.com/Samson-Mano/Delaunay-triangulation)
- [GitHub/rexdwyer/DelaunayTriangulation](https://github.com/rexdwyer/DelaunayTriangulation)
- [GitHub/Bathlamos/delaunay-triangulation](https://github.com/Bathlamos/delaunay-triangulation)
- [GitHub/eloraiby/delaunay](https://github.com/eloraiby/delaunay)
- [GitHub/bl4ckb0ne/delaunay-triangulation](https://github.com/bl4ckb0ne/delaunay-triangulation)
- [Wikipedia, *Delaunay Triangulation*](https://en.wikipedia.org/wiki/Delaunay_triangulation)
- [Wikipedia, *Bowyer-Watson Algorithm*](https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm)