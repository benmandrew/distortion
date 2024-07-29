cmake_minimum_required(VERSION 3.10)

project(Distortion VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(lodepng)
add_subdirectory(inc)
add_subdirectory(src)

target_include_directories(
    Distortion PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/lodepng"
)
