#include "filter.h"

class Kernel {
   public:
    int w, h;
    std::vector<int> data;

    Kernel(std::vector<int> kernel, int w, int h);
};

Kernel::Kernel(std::vector<int> kernel, int w, int h) {
    this->w = w;
    this->h = h;
    this->data = kernel;
}

Filter::Filter(std::vector<ivec4> data, int w, int h) {
    this->w = w;
    this->h = h;
    this->data = data;
}

Filter& Filter::abs() {
    auto f = [](ivec4& v) { return v.v_abs(); };
    std::transform(data.begin(), data.end(), data.begin(), f);
    return *this;
}

Image Filter::to_image() { return Image(data, w, h); }

Image Filter::to_min_zero_image() const {
    std::vector<ivec4> out(w * h);
    for (int i = 0; i < data.size(); i++) {
        out[i] = data[i].v_min_zero();
    }
    return Image(out, w, h);
}

Filter& Filter::scale(double c) {
    auto f = [c](ivec4& v) { return v.scale(c); };
    std::transform(data.begin(), data.end(), data.begin(), f);
    return *this;
}

#include <numeric>

Filter apply_filter(const Image& image, const Kernel& kernel) {
    auto data = std::vector<ivec4>(image.w * image.h);
    double kmag = 0.0;
    for (int v : kernel.data) {
        kmag += static_cast<double>(abs(v));
    }
    for (int j = 0; j < image.h; j++) {
        for (int i = 0; i < image.w; i++) {
            int idx = j * image.w + i;
            if (i < kernel.w / 2 or i >= image.w - kernel.w / 2 or
                j < kernel.h / 2 or j >= image.h - kernel.h / 2) {
                data[idx] = ivec4::zero;
            } else {
                auto v = ivec4::zero;
                for (int kj = 0; kj < kernel.h; kj++) {
                    for (int ki = 0; ki < kernel.w; ki++) {
                        int kidx = idx + (kj - kernel.h / 2) * image.w +
                                   (ki - kernel.w / 2);
                        v = v.add(image.data[kidx].scale(
                            kernel.data[kj * kernel.w + ki]));
                    }
                }
                data[idx] = v.scale(1.0 / kmag);
            }
            data[idx].a = 255;
        }
    }
    return Filter(data, image.w, image.h);
}

Filter Filter::sobel_horizontal(const Image& image) {
    std::vector<int> k = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    return apply_filter(image, Kernel(k, 3, 3));
}

Filter Filter::sobel_vertical(const Image& image) {
    std::vector<int> k = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    return apply_filter(image, Kernel(k, 3, 3));
}

Filter Filter::laplacian(const Image& image) {
    std::vector<int> k = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    return apply_filter(image, Kernel(k, 3, 3));
}

Filter Filter::box(const Image& image) {
    std::vector<int> k = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    return apply_filter(image, Kernel(k, 3, 3));
}

Filter Filter::gaussian(const Image& image) {
    std::vector<int> k = {1,  4, 7, 4,  1,  4,  16, 26, 16, 4, 7, 26, 41,
                          26, 7, 4, 16, 26, 16, 4,  1,  4,  7, 4, 1};
    return apply_filter(image, Kernel(k, 5, 5));
}
