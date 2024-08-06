#include "relblock.h"

RelBlock::RelBlock(const Image& image, int block_width) {
    this->w = image.w;
    this->h = image.h;
    this->block_width = block_width;
    this->centers = get_centers(image.data);
    this->rel_blocks = get_relative_blocks(image.data);
}

std::vector<ivec4> RelBlock::get_centers(const std::vector<ivec4>& data) const {
    int center_offset = block_width / 2;
    int n_blocks_w = w / block_width;
    int n_blocks_h = h / block_width;
    std::vector<ivec4> centers(n_blocks_h * n_blocks_w);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            int target_i = j * n_blocks_w + i;
            int source_i = (j * block_width + center_offset) * w +
                           i * block_width + center_offset;
            centers[target_i] = data[source_i];
        }
    }
    return centers;
}

std::vector<ivec4> RelBlock::get_relative_blocks(
    const std::vector<ivec4>& data) const {
    int n_blocks_w = w / block_width;
    int n_blocks_h = h / block_width;
    std::vector<ivec4> rel_blocks(data.size(), ivec4::zero);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            int c_idx = j * n_blocks_w + i;
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    int r_idx = (j * block_width + l) * w + i * block_width + k;
                    rel_blocks[r_idx] = data[r_idx].sub(centers[c_idx]);
                }
            }
        }
    }
    return rel_blocks;
}

Image RelBlock::to_image() const {
    int n_blocks_w = w / block_width;
    int n_blocks_h = h / block_width;
    std::vector<ivec4> data(rel_blocks.size(), ivec4::zero);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            int c_idx = j * n_blocks_w + i;
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    int r_idx = (j * block_width + l) * w + i * block_width + k;
                    data[r_idx] = rel_blocks[r_idx].add(centers[c_idx]);
                }
            }
        }
    }
    return Image(data, w, h);
}

Image RelBlock::rel_to_image() const {
    std::vector<ivec4> data(rel_blocks.size(), ivec4::zero);
    for (int i = 0; i < rel_blocks.size(); i++) {
        data[i] = rel_blocks[i].v_abs();
        data[i].a = 255;
    }
    return Image(data, w, h);
}
