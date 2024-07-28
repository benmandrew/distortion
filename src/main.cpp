#include <iostream>

#include "main.h"
#include "lodepng.h"

#include "img.cpp"

// PNG file to RGBA pixel data
Image decode(const char *filename) {
    Image png;
    Image image;
    u_int width, height;
    u_int error = lodepng::load_file(png, filename);
    if (!error) {
        error = lodepng::decode(image, width, height, png);
    }
    if (error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
    return image;
}

// RGBA pixel data to PNG file
void encode(const char* filename, Image &image, u_int width, u_int height) {
    Image png;
    u_char error = lodepng::encode(png, image, width, height);
    if (!error) {
        lodepng::save_file(png, filename);
    }
    if (error) {
        std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
    }
}

void output_version(char *argv[]) {
    std::cout << argv[0] << " Version "
        << Distortion_VERSION_MAJOR << "."
        << Distortion_VERSION_MINOR << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        output_version(argv);
        return 1;
    }
    Image v = decode(argv[1]);
    Image w = posterise(v, true); 
    encode("resources/out.png", w, 10, 10);
    return 0;
}
