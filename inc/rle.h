#ifndef RLE_H
#define RLE_H

#include <random>

#include "image.h"

class Rle {
  public:
    std::vector<size_t> lengths;
    ImgData colours;
    u_int w, h;

    Rle(Image &image);
    Image to_image();

    void encode(ImgData &data);
    static ImgData decode(std::vector<size_t> &data, u_int w, u_int h);

    void add_noise(double stddev);
    void add_noise_rows(double stddev);
};

#endif
