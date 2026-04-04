project('lodepng', 'cpp')

lodepng_lib = static_library(
  'lodepng',
  'lodepng.cpp',
)

lodepng_dep = declare_dependency(
  link_with: lodepng_lib,
  include_directories: include_directories('.'),
)
