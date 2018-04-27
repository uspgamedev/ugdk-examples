add_ugdk_executable(example-draw-texture draw-texture.cc)
target_compile_definitions(example-draw-texture PRIVATE EXAMPLE_LOCATION="${CMAKE_CURRENT_SOURCE_DIR}")
