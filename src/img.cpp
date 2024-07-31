#include "defs.h"

#include <random>

class Image {
  public:
    ImgData data;
    u_int w, h;

    Image(u_int w, u_int h);
    Image(ImgData &data, u_int w, u_int h);

    size_t size();
    Image posterise(bool ignore_alpha);
    Image streak_down();
};

Image::Image(u_int w, u_int h) {
    this->data = ImgData(w * h * 4);
    this->w = w;
    this->h = h;
}

Image::Image(ImgData &data, u_int w, u_int h) {
    this->data = data;
    this->w = w;
    this->h = h;
}

size_t Image::size() {
    return w * h;
}

#define POST_LEVELS 8
#define POST_COEFF (u_char)(256 / POST_LEVELS)

u_char posterise_value(u_char v) {
    if (v == 255) {
        return v;
    }
    return (v / POST_COEFF) * POST_COEFF;
}

Image Image::posterise(bool ignore_alpha = true) {
    Image out(w, h);
    for (int i = 0; i < data.size(); i++) {
        if (ignore_alpha && i % 4 == 3) {
            out.data[i] = data[i];
        } else {
            out.data[i] = posterise_value(data[i]);
        }
    }
    return out;
}

double luminance(u_char r, u_char g, u_char b) {
    return 0.2126 * double(r) + 0.7152 * double(g) + 0.0722 * double(b);
}

Image Image::streak_down() {
    Image out(w, h);
    for (int j = h - 1; j >= 0; j--) {
        for (int i = 0; i < w; i++) {
            size_t idx = (j * w + i) * 4;
            u_char r = data[idx];
            u_char g = data[idx + 1];
            u_char b = data[idx + 2];
            u_char a = data[idx + 3];
            u_char streak_len = std::max(u_char(1), u_char(exp2(luminance(r, g,
                                         b) / 36.0)));
            for (int k = 0; k < streak_len; k++) {
                if (j + k >= h) {
                    break;
                }
                size_t s_idx = ((j + k) * w + i) * 4;
                out.data[s_idx] = r;
                out.data[s_idx + 1] = g;
                out.data[s_idx + 2] = b;
                out.data[s_idx + 3] = a;
            }
        }
    }
    return out;
}

// Run-length encoded image
class Rle {
  public:
    std::vector<size_t> data;
    u_int w, h;

    Rle(Image &image);
    Image to_image();

    static std::vector<size_t> encode(ImgData &data);
    static ImgData decode(std::vector<size_t> &data, u_int w, u_int h);

    void add_noise(double stddev);
    void add_noise_rows(double stddev);
};

std::vector<size_t> Rle::encode(ImgData &data) {
    u_char r = data[0];
    u_char g = data[1];
    u_char b = data[2];
    u_char a = data[3];
    size_t runlength = 1;
    size_t i = 1;
    size_t len = data.size() / 4;
    std::vector<size_t> out;
    while (i < len) {
        if (r != data[i * 4]
                || g != data[i * 4 + 1]
                || b != data[i * 4 + 2]
                || a != data[i * 4 + 3]) {
            out.insert(out.end(), {runlength, r, g, b, a});
            runlength = 1;
            r = data[i * 4];
            g = data[i * 4 + 1];
            b = data[i * 4 + 2];
            a = data[i * 4 + 3];
        } else {
            runlength++;
        }
        i++;
    }
    out.insert(out.end(), {runlength, r, g, b, a});
    return out;
}

ImgData Rle::decode(std::vector<size_t> &data, u_int w, u_int h) {
    ImgData out;
    size_t rl_len = data.size() / 5;
    for (int i = 0; i < rl_len; i++) {
        size_t runlength = data[i * 5];
        for (int j = 0; j < runlength; j++) {
            u_char r = data[i * 5 + 1];
            u_char g = data[i * 5 + 2];
            u_char b = data[i * 5 + 3];
            u_char a = data[i * 5 + 4];
            out.insert(out.end(), {r, g, b, a});
        }
    }
    return out;
}

Rle::Rle(Image &image) {
    this->data = encode(image.data);
    this->w = image.w;
    this->h = image.h;
}

Image Rle::to_image() {
    ImgData d = decode(data, w, h);
    return Image(d, w, h);
}

void Rle::add_noise(double stddev = 1.0) {
    std::default_random_engine gen;
    std::normal_distribution<double> dist(0.0, stddev);
    size_t rl_len = data.size() / 5;
    size_t target_len = w * h;
    size_t total_len = 0;
    for (int i = 0; i < rl_len; i++) {
        int runlength = int(data[i * 5]);
        int offset = int(dist(gen));
        if (runlength < -offset) {
            data[i * 5] = 0;
        } else {
            data[i * 5] = size_t(runlength + offset);
        }
        total_len += data[i * 5];
        if (total_len > target_len) {
            data[i * 5] -= total_len - target_len;
            break;
        }
    }
    if (total_len < target_len) {
        data[data.size() - 5] += target_len - total_len;
    }
}

void Rle::add_noise_rows(double stddev = 1.0) {
    std::default_random_engine gen;
    std::normal_distribution<double> dist(0.0, stddev);
    size_t n_prev_runs = 0;
    for (int j = 0; j < h; j++) {
        size_t runs_in_row = 0;
        size_t row_len = 0;
        while (row_len < w) {
            row_len += data[(n_prev_runs + runs_in_row) * 5];
            runs_in_row++;
        }
        row_len = 0;
        for (int i = 0; i < runs_in_row; i++) {
            size_t idx = (n_prev_runs + i) * 5;
            int runlength = int(data[idx]);
            int offset = int(dist(gen));
            if (runlength < -offset) {
                data[idx] = 0;
            } else {
                data[idx] = size_t(runlength + offset);
            }
            row_len += data[idx];
            if (row_len > w) {
                row_len -= data[idx];
                data[idx] = 0;
            }
        }
        if (row_len <= w) {
            data[(n_prev_runs + runs_in_row) * 5 - 5] += w - row_len;
        }
        n_prev_runs += runs_in_row;
    }
}

class RelBlock {
  public:
    u_int w, h;
    std::vector<size_t> centers;
    std::vector<int> rel_blocks;
    u_int block_width;

    RelBlock(Image &image, u_int block_width);
    Image to_image();

    std::vector<size_t> get_centers(ImgData &data);
    std::vector<int> get_relative_blocks(ImgData &data);

    Image rel_to_image();
};

RelBlock::RelBlock(Image &image, u_int block_width) {
    this->w = image.w;
    this->h = image.h;
    this->block_width = block_width;
    this->centers = get_centers(image.data);
    this->rel_blocks = get_relative_blocks(image.data);
}

std::vector<size_t> RelBlock::get_centers(ImgData &data) {
    u_int center_offset = block_width / 2;
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    std::vector<size_t> centers(n_blocks_h * n_blocks_w * 4);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t target_i = j * n_blocks_w + i;
            size_t source_i = (j * block_width + center_offset) * w + i * block_width +
                              center_offset;
            centers[target_i * 4] = data[source_i * 4];
            centers[target_i * 4 + 1] = data[source_i * 4 + 1];
            centers[target_i * 4 + 2] = data[source_i * 4 + 2];
            centers[target_i * 4 + 3] = data[source_i * 4 + 3];
        }
    }
    return centers;
}

std::vector<int> RelBlock::get_relative_blocks(ImgData &data) {
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    std::vector<int> rel_blocks(data.size(), 0);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t c_idx = (j * n_blocks_w + i) * 4;
            int c_r = (int)centers[c_idx];
            int c_g = (int)centers[c_idx + 1];
            int c_b = (int)centers[c_idx + 2];
            int c_a = (int)centers[c_idx + 3];
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    size_t r_idx = ((j * block_width + l) * w + i * block_width + k) * 4;
                    rel_blocks[r_idx] = int(data[r_idx]) - c_r;
                    rel_blocks[r_idx + 1] = int(data[r_idx + 1]) - c_g;
                    rel_blocks[r_idx + 2] = int(data[r_idx + 2]) - c_b;
                    rel_blocks[r_idx + 3] = int(data[r_idx + 3]) - c_a;
                }
            }
        }
    }
    return rel_blocks;
}

Image RelBlock::to_image() {
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    ImgData data(rel_blocks.size(), 0);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t c_idx = (j * n_blocks_w + i) * 4;
            size_t c_r = centers[c_idx];
            size_t c_g = centers[c_idx + 1];
            size_t c_b = centers[c_idx + 2];
            size_t c_a = centers[c_idx + 3];
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    size_t r_idx = ((j * block_width + l) * w + i * block_width + k) * 4;
                    data[r_idx] = size_t(rel_blocks[r_idx]) + c_r;
                    data[r_idx + 1] = size_t(rel_blocks[r_idx + 1]) + c_g;
                    data[r_idx + 2] = size_t(rel_blocks[r_idx + 2]) + c_b;
                    data[r_idx + 3] = size_t(rel_blocks[r_idx + 3]) + c_a;
                }
            }
        }
    }
    return Image(data, w, h);
}

Image RelBlock::rel_to_image() {
    ImgData data(rel_blocks.size(), 0);
    for (int i = 0; i < rel_blocks.size(); i++) {
        data[i] = u_char(abs(rel_blocks[i]));
        if (i % 4 == 3) {
            data[i] = 255;
        }
    }
    return Image(data, w, h);
}
