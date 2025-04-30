#include "image.h"

Image::Image(int w, int h)
    : w{w}, h{h}, data{static_cast<size_t>(w * h)} {}

Image::Image(const std::vector<ivec4>& data, int w, int h)
    : w{w}, h{h}, data{data} {}

Image::Image(const std::vector<uvec4>& data, int w, int h)
    : w{w}, h{h}, data{data.size()} {
    std::transform(data.cbegin(), data.cend(),
                   this->data.begin(), uvec4_to_ivec4);
}

const ivec4& Image::get_px(int x, int y) const {
    x = std::clamp(x, 0, w - 1);
    y = std::clamp(y, 0, h - 1);
    return data[y * w + x];
}

ivec4& Image::get_px(int x, int y) {
    return const_cast<ivec4&>(
        const_cast<const Image*>(this)->get_px(x, y));
}

void Image::set_px(int x, int y, const ivec4& v) {
    size_t i = y * w + x;
    data[i].r = v.r;
    data[i].g = v.g;
    data[i].b = v.b;
    data[i].a = v.a;
}

Image Image::duplicate() const { return Image(data, w, h); }

#define POSTERISATION_LEVELS 8
#define POSTERISATION_COEFF \
    (unsigned char)(256 / POSTERISATION_LEVELS)

unsigned char posterise_value(unsigned char v) {
    if (v == 255) {
        return v;
    }
    return (v / POSTERISATION_COEFF) * POSTERISATION_COEFF;
}

Image& Image::posterise(bool ignore_alpha = true) {
    std::vector<ivec4> out(data.size());
    for (size_t i = 0; i < data.size(); i++) {
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
    data = std::move(out);
    return *this;
}

double get_streak_len(double lum) {
    return pow(lum / 256.0, 2.0) * 10;
}

Image& Image::streak(
    const std::vector<int> h_iter,
    const std::vector<int> v_iter,
    const std::function<std::optional<int>(int, int, int,
                                           int, int)>
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
            unsigned char streak_len =
                get_streak_len(measure(v_m));
            streak_len = std::max(
                static_cast<unsigned char>(1),
                static_cast<unsigned char>(streak_len));
            for (int k = 0; k < streak_len; k++) {
                std::optional<int> s_idx_op =
                    get_streak_idx(w, h, i, j, k);
                if (not s_idx_op.has_value()) {
                    break;
                }
                int s_idx = s_idx_op.value();
                out[s_idx] = v_o;
            }
        }
    }
    data = std::move(out);
    return *this;
}

Image& Image::streak_down(
    const std::optional<Image>& measure_source) {
    auto h_iter = std::vector<int>(w);
    auto v_iter = std::vector<int>(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.rbegin(), v_iter.rend(), 0);
    auto get_streak_pos = [](int w, int h, int i, int j,
                             int k) -> std::optional<int> {
        if (j + k >= h) {
            return std::nullopt;
        }
        return (j + k) * w + i;
    };
    auto measure = [](const ivec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure,
                  measure_source);
}

Image& Image::streak_up(
    const std::optional<Image>& measure_source) {
    auto h_iter = std::vector<int>(w);
    auto v_iter = std::vector<int>(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h [[gnu::unused]],
                             int i, int j,
                             int k) -> std::optional<int> {
        if (j - k < 0) {
            return std::nullopt;
        }
        return (j - k) * w + i;
    };
    auto measure = [](const ivec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure,
                  measure_source);
}

Image& Image::streak_left(
    const std::optional<Image>& measure_source) {
    auto h_iter = std::vector<int>(w);
    auto v_iter = std::vector<int>(h);
    std::iota(h_iter.begin(), h_iter.end(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h [[gnu::unused]],
                             int i, int j,
                             int k) -> std::optional<int> {
        if (i - k < 0) {
            return std::nullopt;
        }
        return j * w + i - k;
    };
    auto measure = [](const ivec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure,
                  measure_source);
}

Image& Image::streak_right(
    const std::optional<Image>& measure_source) {
    auto h_iter = std::vector<int>(w);
    auto v_iter = std::vector<int>(h);
    std::iota(h_iter.rbegin(), h_iter.rend(), 0);
    std::iota(v_iter.begin(), v_iter.end(), 0);
    auto get_streak_pos = [](int w, int h [[gnu::unused]],
                             int i, int j,
                             int k) -> std::optional<int> {
        if (i + k >= w) {
            return std::nullopt;
        }
        return j * w + i + k;
    };
    auto measure = [](const ivec4& v) {
        return v.luminance();
    };
    return streak(h_iter, v_iter, get_streak_pos, measure,
                  measure_source);
}

Image& Image::apply_function(
    const std::function<ivec4(ivec4&)> f) {
    std::transform(data.begin(), data.end(), data.begin(),
                   f);
    return *this;
}

Image& Image::add(const Image& other, double other_ratio) {
    std::vector<ivec4> out(data.size());
    for (size_t i = 0; i < data.size(); i++) {
        out[i] = (data[i].scale(1.0 - other_ratio))
                     .add(other.data[i].scale(other_ratio));
    }
    data = std::move(out);
    return *this;
}

Image& Image::half_size() {
    int hw = w / 2, hh = h / 2;
    std::vector<ivec4> out(hw * hh);
    for (int j = 0; j < hh; j++) {
        for (int i = 0; i < hw; i++) {
            ivec4& tl = data[(2 * j) * w + (2 * i)];
            ivec4& tr = data[(2 * j) * w + (2 * i + 1)];
            ivec4& bl = data[(2 * j + 1) * w + (2 * i)];
            ivec4& br = data[(2 * j + 1) * w + (2 * i + 1)];
            out[j * hw + i] =
                tl.add(tr).add(bl).add(br).scale(0.25);
        }
    }
    w = hw;
    h = hh;
    data = std::move(out);
    return *this;
}

Image& Image::abs() {
    auto f = [](ivec4& v) { return v.abs(); };
    return apply_function(f);
}

Image& Image::clamp_zero() {
    auto f = [](ivec4& v) { return v.min_zero(); };
    return apply_function(f);
}

Image& Image::hard_clamp(double max) {
    auto f = [max](ivec4& v) { return v.hard_clamp(max); };
    return apply_function(f);
}

Image& Image::smooth_clamp(double half, double max) {
    auto f = [half, max](ivec4& v) {
        return v.smooth_clamp(half, max);
    };
    return apply_function(f);
}

Image& Image::modulo(int mod) {
    auto f = [mod](ivec4& v) { return v.modulo(mod); };
    return apply_function(f);
}

Image& Image::scale(double c) {
    auto f = [c](ivec4& v) { return v.scale(c); };
    return apply_function(f);
}

Image& Image::remove_red() {
    auto f = [](ivec4& v) {
        v.r = 0;
        return v;
    };
    return apply_function(f);
}

Image& Image::remove_green() {
    auto f = [](ivec4& v) {
        v.g = 0;
        return v;
    };
    return apply_function(f);
}

Image& Image::remove_blue() {
    auto f = [](ivec4& v) {
        v.b = 0;
        return v;
    };
    return apply_function(f);
}

Image& Image::black_and_white() {
    auto f = [](ivec4& v) {
        int lum = static_cast<int>(v.luminance());
        return vec4{lum, lum, lum, 255};
    };
    return apply_function(f);
}

Image& Image::rgb_to_hsv() {
    auto f = [](ivec4& v) { return v.rgb_to_hsv(); };
    return apply_function(f);
}

Image& Image::hsv_to_rgb() {
    auto f = [](ivec4& v) { return v.hsv_to_rgb(); };
    return apply_function(f);
}

#include <numeric>

Image& Image::apply_filter(const Kernel& k,
                           bool normalise) {
    auto out = std::vector<ivec4>(w * h);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int idx = j * w + i;
            out[idx] = ivec4::zero;
            for (int kj = 0; kj < k.h; kj++) {
                for (int ki = 0; ki < k.w; ki++) {
                    int kx = i + ki - k.w / 2;
                    int ky = j + kj - k.h / 2;
                    out[idx] =
                        out[idx].add(get_px(kx, ky).scale(
                            k.get_px(ki, kj)));
                }
            }
            out[idx].a = 255;
        }
    }
    if (normalise) {
        double kmag = static_cast<double>(k.abs_mag());
        for (ivec4& v : out) {
            v = v.scale(1.0 / kmag);
        }
    }
    data = std::move(out);
    return *this;
}

Image& Image::sobel_horizontal(bool normalise) {
    auto k = std::vector<int>{-1, 0, 1, -2, 0, 2, -1, 0, 1};
    return apply_filter(Kernel(k, 3, 3), normalise);
}

Image& Image::sobel_vertical(bool normalise) {
    auto k = std::vector<int>{-1, -2, -1, 0, 0, 0, 1, 2, 1};
    return apply_filter(Kernel(k, 3, 3), normalise);
}

Image& Image::laplacian3(bool normalise) {
    auto k =
        std::vector<int>{-1, -1, -1, -1, 8, -1, -1, -1, -1};
    return apply_filter(Kernel(k, 3, 3), normalise);
}

Image& Image::laplacian5(bool normalise) {
    auto k = std::vector<int>{
        0,  0,  -1, 0,  0,  0,  -1, -2, -1, 0,  -1, -2, 16,
        -2, -1, 0,  -1, -2, -1, 0,  0,  0,  -1, 0,  0};
    return apply_filter(Kernel(k, 5, 5), normalise);
}

Image& Image::box(bool normalise) {
    auto k = std::vector<int>{1, 1, 1, 1, 1, 1, 1, 1, 1};
    return apply_filter(Kernel(k, 3, 3), normalise);
}

Image& Image::gaussian(bool normalise) {
    auto k = std::vector<int>{
        1,  4, 7, 4,  1,  4,  16, 26, 16, 4, 7, 26, 41,
        26, 7, 4, 16, 26, 16, 4,  1,  4,  7, 4, 1};
    return apply_filter(Kernel(k, 5, 5), normalise);
}
