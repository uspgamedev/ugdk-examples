cmake_minimum_required(VERSION 2.8.11)
project(ugdk-examples)

find_package(Git)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/ugdk/CMakeLists.txt")
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    else(GIT_FOUND)
        message(FATAL_ERROR "Git submodule 'externals/ugdk' not initialized and git not found.")
    endif(GIT_FOUND)
endif()
add_subdirectory(externals/ugdk)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(blank-window)
add_subdirectory(draggable-box)
add_subdirectory(keyboard-box)
add_subdirectory(joystick-box)
add_subdirectory(joystick-display)
add_subdirectory(text)
add_subdirectory(text-from-files)
add_subdirectory(multiple-windows)
add_subdirectory(dynamic-windows)

