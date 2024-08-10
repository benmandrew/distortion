#ifndef RELBLOCK_H
#define RELBLOCK_H

#include "image.h"

class RelBlock {
   public:
    int w, h;
    std::vector<ivec4> centers;
    std::vector<ivec4> rel_blocks;
    int block_width;

    RelBlock(const Image& image, int block_width);
    Image to_image() const;
    Image rel_to_image() const;

    std::vector<ivec4> get_centers(
        const std::vector<ivec4>& data) const;
    std::vector<ivec4> get_relative_blocks(
        const std::vector<ivec4>& data) const;
};

#endif
