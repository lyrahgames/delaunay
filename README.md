# Delaunay Triangulation

A simple implementation of the Bowyer-Watson algorithm to construct the Delaunay triangulation of a given set of points.
It was put into an easy-to-use C++ header-only library currently based on the [build2](https://build2.org/) build system.


![](https://img.shields.io/github/languages/top/lyrahgames/delaunay.svg?style=for-the-badge)
![](https://img.shields.io/github/languages/code-size/lyrahgames/delaunay.svg?style=for-the-badge)
![](https://img.shields.io/github/repo-size/lyrahgames/delaunay.svg?style=for-the-badge)
![](https://img.shields.io/github/license/lyrahgames/delaunay.svg?style=for-the-badge&color=blue)
<!-- [![Website lyrahgames.github.io/pxart](https://img.shields.io/website/https/lyrahgames.github.io/pxart.svg?down_message=offline&label=Documentation&style=for-the-badge&up_color=blue&up_message=online)](https://lyrahgames.github.io/pxart) -->

<b>
<table>
    <tr>
        <td>
            master
        </td>
        <td>
            <a href="https://github.com/lyrahgames/delaunay">
                <img src="https://img.shields.io/github/last-commit/lyrahgames/delaunay/master.svg?logo=github&logoColor=white">
            </a>
        </td>    
        <td>
            <!-- <a href="https://circleci.com/gh/lyrahgames/delaunay/tree/master"><img src="https://circleci.com/gh/lyrahgames/delaunay/tree/master.svg?style=svg"></a> -->
        </td>
        <td>
            <!-- <a href="https://codecov.io/gh/lyrahgames/delaunay">
              <img src="https://codecov.io/gh/lyrahgames/delaunay/branch/master/graph/badge.svg" />
            </a> -->
        </td>
    </tr>
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
    </tr>
</table>
</b>

## Author
- Markus Pawellek (lyrahgames@mailbox.org)

## Requirements
- C++17
- [build2](https://build2.org/)

## Supported Platforms
- Operating System: Linux | Windows | MacOS
- Compiler: GCC | Clang | MSVC | MinGW
- Build System: [build2](https://build2.org/)

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

## Benchmarks

## API

## Features

## Background

## Implementation Details

## References

## Further Reading
