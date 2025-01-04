project('raytracer', 'cpp',
  default_options: ['cpp_std=c++11'],
  version: '0.1',
)

target = get_option('target')
cpp = meson.get_compiler('cpp')

if target == 'linux'
  sdl2_dep = dependency('sdl2', static: true, required: true)
  sdl2_image_dep = dependency('SDL2_image', static: true, required: true)

  sdl2_inc = include_directories('/usr/include/SDL2')
  sdl2_image_inc = include_directories('/usr/include/SDL2')

  # Compile ImGui into a static library
  imgui_sources = files('lib/imgui/imgui.cpp', 'lib/imgui/imgui_draw.cpp', 'lib/imgui/imgui_demo.cpp', 'lib/imgui/imgui_tables.cpp', 'lib/imgui/imgui_widgets.cpp', 'lib/imgui/backends/imgui_impl_sdl2.cpp', 'lib/imgui/backends/imgui_impl_sdlrenderer2.cpp', 'lib/imgui/misc/cpp/imgui_stdlib.cpp')
  imgui_inc = include_directories('lib/imgui', 'lib/imgui/backends')
  imgui_lib = static_library('imgui', imgui_sources, include_directories : [imgui_inc, sdl2_inc, sdl2_image_inc])

  subdir('src')

  executable('raytracer', [src_files, gui_files, raytracer_files, util_files],
    include_directories: ['lib/imgui', 'lib/imgui/backends', 'src', sdl2_inc, sdl2_image_inc],
    dependencies: [
      sdl2_dep,
      sdl2_image_dep
    ],
    link_with: [imgui_lib],
    install: true,
  )
#-s TOTAL_MEMORY=67108864
elif target == 'wasm'
  cpp_args = [
    '--use-port=sdl2',
    '--use-port=sdl2_image:formats=png,jpg',
    '--use-port=libpng',
    '--use-port=libjpeg',
    '-sTOTAL_MEMORY=67108864',
    '-sALLOW_MEMORY_GROWTH=1',
    '-O2',
  ]
  cpp_link_args = [
    '-o', 'index.html',
    '--use-port=sdl2',
    '--use-port=sdl2_image:formats=png,jpg',
    '--use-port=libpng',
    '--use-port=libjpeg',
    '--use-preload-plugins',
    '--shell-file', 'template.html',
    '-sTOTAL_MEMORY=67108864',
    '-sALLOW_MEMORY_GROWTH=1',
    '-sEXPORTED_RUNTIME_METHODS=["ccall","cwrap"]',
    '-sFORCE_FILESYSTEM=1',
    '-O2',
  ]
  add_project_arguments(cpp_args, language: 'cpp')
  add_project_link_arguments(cpp_link_args, language: 'cpp')

  # Compile ImGui into a static library
  imgui_sources = files('lib/imgui/imgui.cpp', 'lib/imgui/imgui_draw.cpp', 'lib/imgui/imgui_demo.cpp', 'lib/imgui/imgui_tables.cpp', 'lib/imgui/imgui_widgets.cpp', 'lib/imgui/backends/imgui_impl_sdl2.cpp', 'lib/imgui/backends/imgui_impl_sdlrenderer2.cpp', 'lib/imgui/misc/cpp/imgui_stdlib.cpp')
  imgui_inc = include_directories('lib/imgui', 'lib/imgui/backends')
  imgui_lib = static_library('imgui', imgui_sources, include_directories : [imgui_inc])

  subdir('src')

  executable('raytracer', [src_files, gui_files, raytracer_files, util_files],
    include_directories: ['lib/imgui', 'lib/imgui/backends', 'src'],
    link_with: [imgui_lib],
    install: true,
  )

endif

