#ifndef RLE_H
#define RLE_H

#include <random>

#include "image.h"

class Rle {
   public:
    std::vector<int> lengths;
    std::vector<ivec4> colours;
    int w, h;

    Rle(const Image& image);
    Image to_image() const;

    void encode(const std::vector<ivec4>& data);

    void add_noise(double stddev);
    void add_noise_rows(double stddev);
};

#endif
