#ifndef RELBLOCK_H
#define RELBLOCK_H

#include "image.h"

class RelBlock {
  public:
    u_int w, h;
    std::vector<vec4> centers;
    std::vector<vec4_T<int>> rel_blocks;
    u_int block_width;

    RelBlock(Image &image, u_int block_width);
    Image to_image();

    std::vector<vec4> get_centers(ImgData &data);
    std::vector<vec4_T<int>> get_relative_blocks(ImgData &data);

    Image rel_to_image();
};

#endif
