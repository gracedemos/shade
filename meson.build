project('shade', 'c')

inc = include_directories('linalg')

sdl3 = dependency('sdl3')
m = meson.get_compiler('c').find_library('m')

executable(
  'shade',
  'src/main.c',
  include_directories: inc,
  dependencies: [sdl3, m],
)
