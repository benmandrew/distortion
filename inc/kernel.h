#ifndef KERNEL_H
#define KERNEL_H

#include <vector>

class Kernel {
   private:
    const std::vector<int> kernel;

   public:
    const int w, h;

    Kernel(std::vector<int> k, int w, int h)
        : kernel{k}, w{w}, h{h} {}

    int get_px(int x, int y) const {
        if (x < 0) {
            x = 0;
        } else if (x >= w) {
            x = w - 1;
        }
        if (y < 0) {
            y = 0;
        } else if (y >= h) {
            y = h - 1;
        }
        return kernel[y * w + x];
    }

    int abs_mag() const {
        int mag = 0;
        for (int v : kernel) {
            mag += std::abs(v);
        }
        return mag;
    }
};

#endif
