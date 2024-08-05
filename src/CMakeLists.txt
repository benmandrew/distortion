include_directories(
    ${Distortion_SOURCE_DIR}/inc
    ${Distortion_SOURCE_DIR}/lodepng
)

add_executable(
    Distortion
    main.cpp
    image.cpp
    rle.cpp
    relblock.cpp
    sobel.cpp
)

target_link_libraries(
    Distortion PUBLIC
    lodepng
)
