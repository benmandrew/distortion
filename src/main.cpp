#include <iostream>

#include "main.h"
#include "lodepng.h"

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
    const double v = std::stod(argv[1]);
    std::cout << "Hello world! " << v << std::endl;
    return 0;
}
