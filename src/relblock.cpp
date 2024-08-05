#include "relblock.h"

RelBlock::RelBlock(const Image& image, u_int block_width) {
    this->w = image.w;
    this->h = image.h;
    this->block_width = block_width;
    this->centers = get_centers(image.data);
    this->rel_blocks = get_relative_blocks(image.data);
}

std::vector<vec4> RelBlock::get_centers(const ImgData& data) const {
    u_int center_offset = block_width / 2;
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    std::vector<vec4> centers(n_blocks_h * n_blocks_w);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t target_i = j * n_blocks_w + i;
            size_t source_i = (j * block_width + center_offset) * w +
                              i * block_width + center_offset;
            centers[target_i] = data[source_i];
        }
    }
    return centers;
}

std::vector<vec4_T<int>> RelBlock::get_relative_blocks(
    const ImgData& data) const {
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    std::vector<vec4_T<int>> rel_blocks(data.size(), vec4_T<int>::zero);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t c_idx = j * n_blocks_w + i;
            vec4_T<int> c = vec4_to_ints(centers[c_idx]);
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    size_t r_idx =
                        (j * block_width + l) * w + i * block_width + k;
                    rel_blocks[r_idx] = vec4_to_ints(data[r_idx]).sub(c);
                }
            }
        }
    }
    return rel_blocks;
}

Image RelBlock::to_image() const {
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    ImgData data(rel_blocks.size(), vec4::zero);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t c_idx = j * n_blocks_w + i;
            vec4 c = centers[c_idx];
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    size_t r_idx =
                        (j * block_width + l) * w + i * block_width + k;
                    data[r_idx] = ints_to_vec4(rel_blocks[r_idx]).add(c);
                }
            }
        }
    }
    return Image(data, w, h);
}

Image RelBlock::rel_to_image() const {
    ImgData data(rel_blocks.size(), vec4::zero);
    for (int i = 0; i < rel_blocks.size(); i++) {
        vec4_T<int> v = rel_blocks[i].v_abs();
        v.a = 255;
        data[i] = ints_to_vec4(v);
    }
    return Image(data, w, h);
}
