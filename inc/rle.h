#ifndef RLE_H
#define RLE_H

#include <random>

#include "image.h"

class Rle {
  public:
    std::vector<size_t> lengths;
    ImgData colours;
    u_int w, h;

    Rle(const Image &image);
    Image to_image() const;

    void encode(const ImgData &data);

    void add_noise(double stddev);
    void add_noise_rows(double stddev);
};

#endif
