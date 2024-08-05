#include "image.h"

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
    return pow(lum / 256.0, 32.0) * 100.0;
}

Image Image::streak(std::vector<int> h_iter, std::vector<int> v_iter,
                    std::function<std::optional<int>(int, int, int, int, int)> get_streak_idx,
                    std::function<int(const vec4&)> measure,
                    const std::optional<Image> &measure_source) {
    Image out(w, h);
    for (int j : v_iter) {
        for (int i : h_iter) {
            size_t idx = j * w + i;
            vec4 &v_o = data[idx];
            vec4 v_m = data[idx];
            if (measure_source.has_value()) {
                v_m = measure_source.value().data[idx];
            }
            u_char streak_len = get_streak_len(measure(v_m));
            streak_len = std::max(static_cast<u_char>(1),
                                  static_cast<u_char>(streak_len));
            for (int k = 0; k < streak_len; k++) {
                std::optional<int> s_idx_op = get_streak_idx(w, h, i, j, k);
                if (not s_idx_op.has_value()) {
                    break;
                }
                int s_idx = s_idx_op.value();
                out.data[s_idx] = v_o;
            }
        }
    }
    return out;
}

Image Image::streak_down(const std::optional<Image> &measure_source) {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.rbegin(), v_iter.rend(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j,
    int k) -> std::optional<int> {
        if (j + k >= h) {
            return std::nullopt;
        }
        return (j + k) * w + i;
    };
    auto measure = [](const vec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image Image::streak_up(const std::optional<Image> &measure_source) {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j,
    int k) -> std::optional<int> {
        if (j - k < 0) {
            return std::nullopt;
        }
        return (j - k) * w + i;
    };
    auto measure = [](const vec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image Image::streak_left(const std::optional<Image> &measure_source) {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j,
    int k) -> std::optional<int> {
        if (i - k < 0) {
            return std::nullopt;
        }
        return j * w + i - k;
    };
    auto measure = [](const vec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image Image::streak_right(const std::optional<Image> &measure_source) {
    std::vector<int> h_iter(w), v_iter(h);
    std::iota(h_iter.rbegin(), h_iter.rend(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j,
    int k) -> std::optional<int> {
        if (i + k >= w) {
            return std::nullopt;
        }
        return j * w + i + k;
    };
    auto measure = [](const vec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}
