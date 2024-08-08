#ifndef KERNEL_H
#define KERNEL_H

#include <vector>

class Kernel {
   public:
    int w, h;
    std::vector<int> kernel;

    Kernel(std::vector<int> k, int w, int h) : kernel{k}, w{w}, h{h} {}
};

#endif
