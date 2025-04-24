#include <queue>

#include "image.h"

struct Pixel {
    u_int32_t x, y;
    double lum;

    Pixel(u_int32_t x, u_int32_t y, double lum)
        : x(x), y(y), lum(lum) {}

    bool operator<(const Pixel& other) const {
        return lum < other.lum;
    }
};

class SortedImage {
   public:
    int w, h;
    std::vector<Pixel> data;

    SortedImage(const Image& image);

    Image& to_image();
};

SortedImage::SortedImage(const Image& image)
    : w(image.w), h(image.h) {
    data = std::vector<Pixel>(w * h);
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = j * w + i;
            data[idx].x = i;
            data[idx].y = j;
            data[idx].lum = image.get_px(i, j).luminance();
        }
    }
}
