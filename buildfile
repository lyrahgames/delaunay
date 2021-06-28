import intf_libs = lyrahgames-xstd%lib{lyrahgames-xstd}

./: lib{lyrahgames-delaunay}: \
  lyrahgames/delaunay/hxx{** -version} \
  lyrahgames/delaunay/hxx{version} \
  $intf_libs
{
  cxx.export.poptions = "-I$out_root" "-I$src_root"
  cxx.export.libs = $intf_libs
}
cxx.poptions =+ "-I$out_root" "-I$src_root"

lyrahgames/delaunay/
{
  hxx{version}: in{version} $src_root/manifest
  {
    dist = true
    clean = ($src_root != $out_root)
  }
}

hxx{**}: install.subdirs = true

./: tests/ manifest doc{README.md AUTHORS.md} legal{COPYING.md}
tests/: install = false