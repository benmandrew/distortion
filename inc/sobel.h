#ifndef SOBEL_H
#define SOBEL_H

#include "image.h"

class Sobel {
  public:
    u_int w, h;
    std::vector<vec4_T<int>> data;

    Image to_image();

    static Sobel new_horizontal(Image &image);
    static Sobel new_vertical(Image &image);

    static std::vector<vec4_T<int>> horizontal(Image &image);
    static std::vector<vec4_T<int>> vertical(Image &image);
};

#endif
