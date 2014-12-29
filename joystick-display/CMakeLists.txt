
add_ugdk_executable(example-joystick-display joystick-display.cc)
target_compile_definitions(example-joystick-display PRIVATE EXAMPLE_LOCATION="${CMAKE_CURRENT_SOURCE_DIR}")
