include_directories(
    ${Distortion_SOURCE_DIR}/inc
    ${Distortion_SOURCE_DIR}/lodepng
)

add_executable(
    Distortion
    main.cpp
    img.cpp
)

target_link_libraries(
    Distortion PUBLIC
    lodepng
)
