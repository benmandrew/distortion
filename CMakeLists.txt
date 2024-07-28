cmake_minimum_required(VERSION 3.10)

project(Distortion VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)
add_subdirectory(inc)

target_include_directories(
    Distortion PUBLIC
    "${PROJECT_BINARY_DIR}"
)
