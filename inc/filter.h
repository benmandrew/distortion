#ifndef FILTER_H
#define FILTER_H

#include "image.h"

class Filter {
   public:
    int w, h;
    std::vector<ivec4> data;

    Filter(std::vector<ivec4> data, int w, int h);

    Filter& abs();
    Image to_image();
    Image to_min_zero_image() const;

    Filter& scale(double c);

    static Filter sobel_horizontal(const Image& image);
    static Filter sobel_vertical(const Image& image);
    static Filter laplacian(const Image& image);
    static Filter box(const Image& image);
    static Filter gaussian(const Image& image);
};

#endif
