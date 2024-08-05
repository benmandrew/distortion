#ifndef SOBEL_H
#define SOBEL_H

#include "image.h"

class Sobel {
   public:
    u_int w, h;
    std::vector<vec4_T<int>> data;

    Image to_image();

    static Sobel new_horizontal(const Image &image);
    static Sobel new_vertical(const Image &image);

    static std::vector<vec4_T<int>> horizontal(const Image &image);
    static std::vector<vec4_T<int>> vertical(const Image &image);
};

#endif
