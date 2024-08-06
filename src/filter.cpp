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

Filter::Filter(std::vector<vec4_T<int>> data, int w, int h) {
    this->w = w;
    this->h = h;
    this->data = data;
}

Image Filter::to_abs_image() const {
    ImgData out(w * h);
    for (int i = 0; i < data.size(); i++) {
        vec4_T<int> v = data[i].v_abs().scale(3);
        out[i] = ints_to_vec4(v);
        out[i].a = 255;
    }
    return Image(out, w, h);
}

Image Filter::to_min_zero_image() const {
    ImgData out(w * h);
    for (int i = 0; i < data.size(); i++) {
        vec4_T<int> v = data[i].v_min_zero();
        out[i] = ints_to_vec4(v);
    }
    return Image(out, w, h);
}

void Filter::scale(double c) {
    for (int i = 0; i < data.size(); i++) {
        data[i].scale(c);
    }
}

#include <numeric>

Filter apply_filter(const Image& image, const Kernel& kernel) {
    std::vector<vec4_T<int>> inter(image.w * image.h);
    auto data = std::vector<vec4_T<int>>(image.w * image.h);
    for (int i = 0; i < image.data.size(); i++) {
        inter[i] = vec4_to_ints(image.data[i]);
    }
    double kmag = 0.0;
    for (int v : kernel.data) {
        kmag += static_cast<double>(abs(v));
    }
    for (int j = 0; j < image.h; j++) {
        for (int i = 0; i < image.w; i++) {
            int idx = j * image.w + i;
            if (i < kernel.w / 2 or i >= image.w - kernel.w / 2 or
                j < kernel.h / 2 or j >= image.h - kernel.h / 2) {
                data[idx] = vec4_T<int>::zero;
            } else {
                auto v = vec4_T<int>::zero;
                for (int kj = 0; kj < kernel.h; kj++) {
                    for (int ki = 0; ki < kernel.w; ki++) {
                        // std::cout << +ki << " " << +kj << " " << (ki -
                        // kernel.h / 2) << " " << (kj - kernel.h / 2) <<
                        // std::endl;
                        int kidx = idx + (kj - kernel.h / 2) * image.w +
                                   (ki - kernel.w / 2);
                        v = v.add(
                            inter[kidx].scale(kernel.data[kj * kernel.w + ki]));
                    }
                }
                data[idx] = v.scale(1.0 / kmag);
                // std::cout << +kmag << " " << +v.r << " " << +data[idx].r <<
                // std::endl;
            }
            data[idx].a = 255;
        }
    }
    return Filter(data, image.w, image.h);
}

Filter Filter::sobel_horizontal(const Image& image) {
    std::vector<int> k = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    return apply_filter(image, Kernel(k, 3, 3));
}

Filter Filter::sobel_vertical(const Image& image) {
    std::vector<int> k = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
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
