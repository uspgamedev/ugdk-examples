
add_ugdk_executable(example-text-from-files text.cc)
target_compile_definitions(example-text-from-files PRIVATE EXAMPLE_LOCATION="${CMAKE_CURRENT_SOURCE_DIR}")
