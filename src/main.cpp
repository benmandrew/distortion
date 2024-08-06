#include <cassert>
#include <chrono>
#include <iostream>

#include "filter.h"
#include "image.h"
#include "lodepng.h"
#include "relblock.h"
#include "rle.h"

ImgData to_imgdata(const std::vector<u_char>& data) {
    assert(data.size() % 4 == 0);
    ImgData out(data.size() / 4);
    for (int i = 0; i < out.size(); i++) {
        out[i] = vec4{
            .r = data[4 * i],
            .g = data[4 * i + 1],
            .b = data[4 * i + 2],
            .a = data[4 * i + 3],
        };
    }
    return out;
}

std::vector<u_char> to_data(const ImgData& data) {
    std::vector<u_char> out(data.size() * 4);
    for (int i = 0; i < data.size(); i++) {
        const vec4& v = data[i];
        out[4 * i] = v.r;
        out[4 * i + 1] = v.g;
        out[4 * i + 2] = v.b;
        out[4 * i + 3] = v.a;
    }
    return out;
}

// PNG file to RGBA pixel data
Image decode(const char* filename) {
    std::vector<u_char> png, image;
    u_int width, height;
    u_int error = lodepng::load_file(png, filename);
    if (!error) {
        error = lodepng::decode(image, width, height, png);
    }
    if (error) {
        std::cerr << "decoder error " << error << ": "
                  << lodepng_error_text(error) << std::endl;
    }
    ImgData i = to_imgdata(image);
    return Image(i, width, height);
}

// RGBA pixel data to PNG file
void encode(const char* filename, const Image& image) {
    std::vector<u_char> png;
    const std::vector<u_char> data = to_data(image.data);
    u_char error = lodepng::encode(png, data, image.w, image.h);
    if (!error) {
        lodepng::save_file(png, filename);
    }
    if (error) {
        std::cerr << "encoder error " << error << ": "
                  << lodepng_error_text(error) << std::endl;
    }
}

void output_help(char* argv[]) {
    std::cout << "Usage: " << argv[0] << " [image.png]" << std::endl;
}

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
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Decoding" << std::endl;
    Image v = decode(argv[1]);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                     .count()
              << "ms" << std::endl;
    // Image x = v.posterise();

    // Rle rle = Rle(v);
    // rle.add_noise_rows(0.6);
    // Image x = rle.to_image();

    std::cout << "Processing" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    Image y = Filter::box(v).to_abs_image();
    Filter lap = Filter::laplacian(y);
    // lap.scale(4.0);
    Image x = lap.to_abs_image();
    // Image x = Filter::box(lapi).to_abs_image();
    // Image x = v.streak_up(z);

    // RelBlock r(v, 249);
    // Image x = r.rel_to_image();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                     .count()
              << "ms" << std::endl;

    std::cout << "Encoding" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    encode("resources/out.png", x);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                     .count()
              << "ms" << std::endl;
    return 0;
}
