#include "img.h"

Image::Image(u_int w, u_int h) {
    this->data = ImgData(w * h);
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
        vec4 *v_o = &data[i];
        vec4 *v_t = &out.data[i];
        v_t->r = posterise_value(v_o->r);
        v_t->g = posterise_value(v_o->g);
        v_t->b = posterise_value(v_o->b);
        if (ignore_alpha) {
            v_t->a = v_o->a;
        } else {
            v_t->a = posterise_value(v_o->a);
        }
    }
    return out;
}

double get_streak_len(double lum) {
    return pow(lum / 256.0, 6.0) * 512.0;
}

Image Image::streak(std::vector<int> h_iter, std::vector<int> v_iter,
                    std::function<std::optional<int>(int, int, int, int, int)> get_streak_idx) {
    Image out(w, h);
    for (int j : v_iter) {
        for (int i : h_iter) {
            size_t idx = j * w + i;
            vec4 *v_o = &data[idx];
            u_char streak_len = get_streak_len(v_o->luminance());
            streak_len = std::max(static_cast<u_char>(1),
                                  static_cast<u_char>(streak_len));
            for (int k = 0; k < streak_len; k++) {
                std::optional<int> s_idx_op = get_streak_idx(w, h, i, j, k);
                if (not s_idx_op.has_value()) {
                    break;
                }
                int s_idx = s_idx_op.value();
                out.data[s_idx] = *v_o;
            }
        }
    }
    return out;
}

Image Image::streak_down() {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.rbegin(), v_iter.rend(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j, int k) -> std::optional<int> {
        if (j + k >= h) {
            return std::nullopt;
        }
        return (j + k) * w + i;
    };
    return streak(h_iter, v_iter, get_streak_pos);
}

Image Image::streak_up() {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j, int k) -> std::optional<int> {
        if (j - k < 0) {
            return std::nullopt;
        }
        return (j - k) * w + i;
    };
    return streak(h_iter, v_iter, get_streak_pos);
}

Image Image::streak_left() {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j, int k) -> std::optional<int> {
        if (i - k < 0) {
            return std::nullopt;
        }
        return j * w + i - k;
    };
    return streak(h_iter, v_iter, get_streak_pos);
}

Image Image::streak_right() {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.rbegin(), h_iter.rend(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j, int k) -> std::optional<int> {
        if (i + k >= w) {
            return std::nullopt;
        }
        return j * w + i + k;
    };
    return streak(h_iter, v_iter, get_streak_pos);
}

// Run-length encoded image
class Rle {
  public:
    std::vector<size_t> lengths;
    ImgData colours;
    u_int w, h;

    Rle(Image &image);
    Image to_image();

    void encode(ImgData &data);
    static ImgData decode(std::vector<size_t> &data, u_int w, u_int h);

    void add_noise(double stddev);
    void add_noise_rows(double stddev);
};

void Rle::encode(ImgData &data) {
    vec4 *v = &data[0];
    size_t runlength = 1;
    size_t i = 1;
    size_t len = data.size();
    std::vector<size_t> lengths;
    ImgData colours;
    while (i < len) {
        vec4 *v_tmp = &data[i];
        if (v->r != v_tmp->r
                || v->g != v_tmp->g
                || v->b != v_tmp->b
                || v->a != v_tmp->a) {
            lengths.push_back(runlength);
            colours.push_back(*v);
            runlength = 1;
        } else {
            runlength++;
        }
        i++;
    }
    lengths.push_back(runlength);
    colours.push_back(*v);
}

Rle::Rle(Image &image) {
    this->w = image.w;
    this->h = image.h;
    encode(image.data);
}

Image Rle::to_image() {
    ImgData out;
    size_t rl_len = lengths.size();
    for (int i = 0; i < rl_len; i++) {
        size_t runlength = lengths[i];
        for (int j = 0; j < runlength; j++) {
            out.push_back(colours[i]);
        }
    }
    return Image(out, w, h);
}

void Rle::add_noise(double stddev = 1.0) {
    std::default_random_engine gen;
    std::normal_distribution<double> dist(0.0, stddev);
    size_t rl_len = lengths.size();
    size_t target_len = w * h;
    size_t total_len = 0;
    for (int i = 0; i < rl_len; i++) {
        int runlength = static_cast<int>(lengths[i]);
        int offset = static_cast<int>(dist(gen));
        if (runlength < -offset) {
            lengths[i] = 0;
        } else {
            lengths[i] = static_cast<size_t>(runlength + offset);
        }
        total_len += lengths[i];
        if (total_len > target_len) {
            lengths[i] -= total_len - target_len;
            break;
        }
    }
    if (total_len < target_len) {
        lengths[lengths.size() - 1] += target_len - total_len;
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
            row_len += lengths[n_prev_runs + runs_in_row];
            runs_in_row++;
        }
        row_len = 0;
        for (int i = 0; i < runs_in_row; i++) {
            size_t idx = n_prev_runs + i;
            int runlength = static_cast<int>(lengths[idx]);
            int offset = static_cast<int>(dist(gen));
            if (runlength < -offset) {
                lengths[idx] = 0;
            } else {
                lengths[idx] = size_t(runlength + offset);
            }
            row_len += lengths[idx];
            if (row_len > w) {
                row_len -= lengths[idx];
                lengths[idx] = 0;
            }
        }
        if (row_len <= w) {
            lengths[n_prev_runs + runs_in_row - 1] += w - row_len;
        }
        n_prev_runs += runs_in_row;
    }
}

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

RelBlock::RelBlock(Image &image, u_int block_width) {
    this->w = image.w;
    this->h = image.h;
    this->block_width = block_width;
    this->centers = get_centers(image.data);
    this->rel_blocks = get_relative_blocks(image.data);
}

std::vector<vec4> RelBlock::get_centers(ImgData &data) {
    u_int center_offset = block_width / 2;
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    std::vector<vec4> centers(n_blocks_h * n_blocks_w);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t target_i = j * n_blocks_w + i;
            size_t source_i = (j * block_width + center_offset) * w + i * block_width +
                              center_offset;
            centers[target_i] = data[source_i];
        }
    }
    return centers;
}

std::vector<vec4_T<int>> RelBlock::get_relative_blocks(ImgData &data) {
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    std::vector<vec4_T<int>> rel_blocks(data.size(), vec4_T<int>::zero);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t c_idx = j * n_blocks_w + i;
            vec4_T<int> c = vec4_to_ints(centers[c_idx]);
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    size_t r_idx = (j * block_width + l) * w + i * block_width + k;
                    rel_blocks[r_idx] = vec4_to_ints(data[r_idx]).sub(c);
                }
            }
        }
    }
    return rel_blocks;
}

Image RelBlock::to_image() {
    size_t n_blocks_w = w / block_width;
    size_t n_blocks_h = h / block_width;
    ImgData data(rel_blocks.size(), vec4::zero);
    for (int j = 0; j < n_blocks_h; j++) {
        for (int i = 0; i < n_blocks_w; i++) {
            size_t c_idx = j * n_blocks_w + i;
            vec4 c = centers[c_idx];
            for (int l = 0; l < block_width; l++) {
                for (int k = 0; k < block_width; k++) {
                    size_t r_idx = (j * block_width + l) * w + i * block_width + k;
                    data[r_idx] = ints_to_vec4(rel_blocks[r_idx]).add(c);
                }
            }
        }
    }
    return Image(data, w, h);
}

Image RelBlock::rel_to_image() {
    ImgData data(rel_blocks.size(), vec4::zero);
    for (int i = 0; i < rel_blocks.size(); i++) {
        vec4_T<int> v = rel_blocks[i].v_abs();
        v.a = 255;
        data[i] = ints_to_vec4(v);
    }
    return Image(data, w, h);
}
