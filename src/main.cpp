#include <iostream>

#include "main.h"
#include "lodepng.h"

// PNG file to RGBA pixel data
std::vector<u_char> decode(const char *filename) {
    std::vector<u_char> png;
    std::vector<u_char> image;
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
void encode(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height) {
    std::vector<unsigned char> png;
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
    std::vector<u_char> v = decode(argv[1]);

    for (u_int i = 0; i < 100; i++) {
        v[4 * i] = v[4 * i] + 100;
    }

    encode("out.png", v, 10, 10);
    return 0;
}
