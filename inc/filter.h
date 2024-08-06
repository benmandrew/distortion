#ifndef FILTER_H
#define FILTER_H

#include "image.h"

class Filter {
   public:
    u_int w, h;
    std::vector<vec4_T<int>> data;

    Filter(std::vector<vec4_T<int>> data, int w, int h);

    Image to_abs_image() const;
    Image to_min_zero_image() const;

    void scale(double c);

    static Filter sobel_horizontal(const Image &image);
    static Filter sobel_vertical(const Image &image);
    static Filter laplacian(const Image &image);
    static Filter box(const Image &image);
    static Filter gaussian(const Image &image);
};

#endif
