add_ugdk_executable(example-trailing-img trailing-img.cc)
target_compile_definitions(example-trailing-img PRIVATE EXAMPLE_LOCATION="${CMAKE_CURRENT_SOURCE_DIR}")
