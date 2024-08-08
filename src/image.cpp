#include "image.h"

Image::Image(int w, int h) {
    this->data = std::vector<ivec4>(w * h);
    this->w = w;
    this->h = h;
}

Image::Image(const std::vector<ivec4>& d, int w, int h) {
    this->data = d;
    this->w = w;
    this->h = h;
}

Image::Image(const std::vector<uvec4>& d, int w, int h) {
    this->data = std::vector<ivec4>(d.size());
    std::transform(d.cbegin(), d.cend(), this->data.begin(), uvec4_to_ivec4);
    this->w = w;
    this->h = h;
}

const ivec4& Image::get_px(int x, int y) const {
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
    return data[y * w + x];
}

ivec4& Image::get_px(int x, int y) {
    return const_cast<ivec4&>(const_cast<const Image*>(this)->get_px(x, y));
}

Image Image::duplicate() const {
    auto out = std::vector<ivec4>{data};
    return Image(out, w, h);
}

#define POSTERISATION_LEVELS 8
#define POSTERISATION_COEFF (u_char)(256 / POSTERISATION_LEVELS)

u_char posterise_value(u_char v) {
    if (v == 255) {
        return v;
    }
    return (v / POSTERISATION_COEFF) * POSTERISATION_COEFF;
}

Image& Image::posterise(bool ignore_alpha = true) {
    std::vector<ivec4> out(data.size());
    for (int i = 0; i < data.size(); i++) {
        const auto& v_o = data[i];
        auto& v_t = out[i];
        v_t.r = posterise_value(v_o.r);
        v_t.g = posterise_value(v_o.g);
        v_t.b = posterise_value(v_o.b);
        if (ignore_alpha) {
            v_t.a = v_o.a;
        } else {
            v_t.a = posterise_value(v_o.a);
        }
    }
    this->data = out;
    return *this;
}

double get_streak_len(double lum) { return pow(lum / 256.0, 2.0) * 1000.0; }

Image& Image::streak(
    const std::vector<int> h_iter, const std::vector<int> v_iter,
    const std::function<std::optional<int>(int, int, int, int, int)>
        get_streak_idx,
    const std::function<int(const ivec4&)> measure,
    const std::optional<Image>& measure_source) {
    std::vector<ivec4> out(data.size());
    for (int j : v_iter) {
        for (int i : h_iter) {
            int idx = j * w + i;
            const auto& v_o = data[idx];
            auto v_m = data[idx];
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
                out[s_idx] = v_o;
            }
        }
    }
    this->data = out;
    return *this;
}

Image& Image::streak_down(const std::optional<Image>& measure_source) {
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
    auto measure = [](const ivec4& v) { return v.luminance(); };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image& Image::streak_up(const std::optional<Image>& measure_source) {
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
    auto measure = [](const ivec4& v) { return v.luminance(); };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image& Image::streak_left(const std::optional<Image>& measure_source) {
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
    auto measure = [](const ivec4& v) { return v.luminance(); };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image& Image::streak_right(const std::optional<Image>& measure_source) {
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
    auto measure = [](const ivec4& v) { return v.luminance(); };
    return streak(h_iter, v_iter, get_streak_pos, measure, measure_source);
}

Image& Image::add(const Image& other, double other_ratio) {
    std::vector<ivec4> out(data.size());
    for (int i = 0; i < data.size(); i++) {
        out[i] = (data[i].scale(1.0 - other_ratio))
                     .add(other.data[i].scale(other_ratio));
    }
    this->data = out;
    return *this;
}

Image& Image::abs() {
    auto f = [](ivec4& v) { return v.v_abs(); };
    std::transform(data.begin(), data.end(), data.begin(), f);
    return *this;
}

Image& Image::clamp_zero() {
    auto f = [](ivec4& v) { return v.v_min_zero(); };
    std::transform(data.begin(), data.end(), data.begin(), f);
    return *this;
}

Image& Image::smooth_clamp(double half, double max) {
    auto f = [half, max](ivec4& v) { return v.smooth_clamp(half, max); };
    std::transform(data.begin(), data.end(), data.begin(), f);
    return *this;
}
Image& Image::scale(double c) {
    auto f = [c](ivec4& v) { return v.scale(c); };
    std::transform(data.begin(), data.end(), data.begin(), f);
    return *this;
}

#include <numeric>

Image& Image::apply_filter(const Kernel& k) {
    auto data = std::vector<ivec4>(w * h);
    double kmag = 0.0;
    for (int v : k.kernel) {
        kmag += static_cast<double>(std::abs(v));
    }
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int idx = j * w + i;
            auto out = ivec4::zero;
            for (int kj = 0; kj < k.h; kj++) {
                for (int ki = 0; ki < k.w; ki++) {
                    int kx = i + ki - k.w / 2;
                    int ky = j + kj - k.h / 2;
                    out =
                        out.add(get_px(kx, ky).scale(k.kernel[kj * k.w + ki]));
                }
            }
            data[idx] = out.scale(1.0 / kmag);
            data[idx].a = 255;
        }
    }
    this->data = data;
    return *this;
}

Image& Image::sobel_horizontal() {
    std::vector<int> k = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    return apply_filter(Kernel(k, 3, 3));
}

Image& Image::sobel_vertical() {
    std::vector<int> k = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    return apply_filter(Kernel(k, 3, 3));
}

Image& Image::laplacian3() {
    std::vector<int> k = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    return apply_filter(Kernel(k, 3, 3));
}

Image& Image::laplacian5() {
    std::vector<int> k = {0,  0,  -1, 0,  0,  0,  -1, -2, -1, 0,  -1, -2, 16,
                          -2, -1, 0,  -1, -2, -1, 0,  0,  0,  -1, 0,  0};
    return apply_filter(Kernel(k, 5, 5));
}

Image& Image::box() {
    std::vector<int> k = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    return apply_filter(Kernel(k, 3, 3));
}

Image& Image::gaussian() {
    std::vector<int> k = {1,  4, 7, 4,  1,  4,  16, 26, 16, 4, 7, 26, 41,
                          26, 7, 4, 16, 26, 16, 4,  1,  4,  7, 4, 1};
    return apply_filter(Kernel(k, 5, 5));
}
