#include <iostream>
#include <cassert>

#include "img.cpp"
#include "main.h"
#include "lodepng.h"

ImgData to_imgdata(std::vector<u_char> &data) {
    assert(data.size() % 4 == 0);
    ImgData out(data.size() / 4);
    for (int i = 0; i < out.size(); i++) {
        out[i] = vec4 {
            .r = data[4 * i],
            .g = data[4 * i + 1],
            .b = data[4 * i + 2],
            .a = data[4 * i + 3],
        };
    }
    return out;
}

std::vector<u_char> to_data(ImgData &data) {
    std::vector<u_char> out(data.size() * 4);
    for (int i = 0; i < data.size(); i++) {
        vec4 &v = data[i];
        out[4 * i] = v.r;
        out[4 * i + 1] = v.g;
        out[4 * i + 2] = v.b;
        out[4 * i + 3] = v.a;
    }
    return out;
}

// PNG file to RGBA pixel data
Image decode(const char *filename) {
    std::vector<u_char> png, image;
    u_int width, height;
    u_int error = lodepng::load_file(png, filename);
    if (!error) {
        error = lodepng::decode(image, width, height, png);
    }
    if (error) {
        std::cerr << "decoder error " << error << ": " << lodepng_error_text(
                      error) << std::endl;
    }
    ImgData i = to_imgdata(image);
    return Image(i, width, height);
}

// RGBA pixel data to PNG file
void encode(const char* filename, Image &image) {
    std::vector<u_char> png;
    std::vector<u_char> data = to_data(image.data);
    u_char error = lodepng::encode(png, data, image.w, image.h);
    if (!error) {
        lodepng::save_file(png, filename);
    }
    if (error) {
        std::cerr << "encoder error " << error << ": "<< lodepng_error_text(
                      error) << std::endl;
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
    // Image x = v.posterise();

    // Rle rle = Rle(v);
    // rle.add_noise_rows(0.6);
    // Image x = rle.to_image();

    // RelBlock r(v, 60);
    // Image x = r.rel_to_image();

    Image x = v.streak_down();

    encode("resources/out.png", x);
    return 0;
}
