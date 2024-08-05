#ifndef RELBLOCK_H
#define RELBLOCK_H

#include "image.h"

class RelBlock {
  public:
    u_int w, h;
    std::vector<vec4> centers;
    std::vector<vec4_T<int>> rel_blocks;
    u_int block_width;

    RelBlock(const Image &image, u_int block_width);
    Image to_image() const;
    Image rel_to_image() const;

    std::vector<vec4> get_centers(const ImgData &data) const;
    std::vector<vec4_T<int>> get_relative_blocks(const ImgData &data) const;
};

#endif
