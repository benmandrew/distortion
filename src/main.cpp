#include <cassert>
#include <chrono>
#include <iostream>

#include "dct.h"
#include "image.h"
#include "lodepng.h"
#include "relblock.h"
#include "rle.h"

std::vector<ivec4> to_vectors(
    const std::vector<unsigned char>& data) {
    assert(data.size() % 4 == 0);
    std::vector<ivec4> out(data.size() / 4);
    for (size_t i = 0; i < out.size(); i++) {
        out[i] = ivec4{
            .r = data[4 * i],
            .g = data[4 * i + 1],
            .b = data[4 * i + 2],
            .a = data[4 * i + 3],
        };
    }
    return out;
}

std::vector<unsigned char> to_data(
    const std::vector<ivec4>& data) {
    std::vector<unsigned char> out(data.size() * 4);
    for (size_t i = 0; i < data.size(); i++) {
        const auto& v = data[i];
        out[4 * i] = v.r;
        out[4 * i + 1] = v.g;
        out[4 * i + 2] = v.b;
        out[4 * i + 3] = v.a;
    }
    return out;
}

// PNG file to RGBA pixel data
Image decode(const char* filename) {
    std::vector<unsigned char> png, image;
    unsigned int width, height;
    unsigned int error = lodepng::load_file(png, filename);
    if (!error) {
        error = lodepng::decode(image, width, height, png);
    }
    if (error) {
        std::cerr << "decoder error " << error << ": "
                  << lodepng_error_text(error) << std::endl;
    }
    auto i = to_vectors(image);
    return Image(i, width, height);
}

// RGBA pixel data to PNG file
void encode(const char* filename, const Image& image) {
    std::vector<unsigned char> png;
    const std::vector<unsigned char> data =
        to_data(image.data);
    unsigned char error =
        lodepng::encode(png, data, image.w, image.h);
    if (!error) {
        lodepng::save_file(png, filename);
    }
    if (error) {
        std::cerr << "encoder error " << error << ": "
                  << lodepng_error_text(error) << std::endl;
    }
}

void output_help(char* argv[]) {
    std::cout << "Usage: " << argv[0] << " [image.png]"
              << std::endl;
}

#define INIT_TIMER()                               \
    auto start =                                   \
        std::chrono::high_resolution_clock::now(); \
    auto end = std::chrono::high_resolution_clock::now();

#define START_TIMER(s)                                 \
    start = std::chrono::high_resolution_clock::now(); \
    std::cout << s << ": ";

#define END_TIMER()                                    \
    end = std::chrono::high_resolution_clock::now();   \
    std::cout << std::chrono::duration_cast<           \
                     std::chrono::milliseconds>(end -  \
                                                start) \
                     .count()                          \
              << "ms" << std::endl;

int main(int argc, char* argv[]) {
    // std::vector<vec4> pl;
    // for (int i = 0; i < 8; i++) {
    //     pl.push_back(vec4::zero);
    // }
    // for (int i = 0; i < 8; i++) {
    //     pl.push_back(vec4::create(128, 128, 128, 128));
    // }
    // Image p = Image(pl, 4, 4);
    // print_image(pl, 4, 4);
    // Image pf = Filter::new_gaussian(p).to_abs_image();
    // print_image(pf.data, 4, 4);

    if (argc < 2) {
        output_help(argv);
        return 1;
    }
    INIT_TIMER();
    START_TIMER("Decoding");
    Image v = decode(argv[1]);
    END_TIMER();

    std::cout << "Input dimensions: " << v.w << "x" << v.h
              << std::endl;

    START_TIMER("Processing");

    auto f = [](ivec4& v) {
        v.r += 63;
        return v;
    };

    Image x = v.half_size().rgb_to_hsv();
    // .apply_function(f)
    // .modulo(256.0)
    // .hsv_to_rgb();
    // .scale(4.0)
    // .modulo(256);

    END_TIMER();

    std::cout << "Output dimensions: " << x.w << "x" << x.h
              << std::endl;

    START_TIMER("Encoding");
    encode("resources/out.png", x);
    END_TIMER();
    return 0;
}
