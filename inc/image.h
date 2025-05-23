#ifndef IMAGE_H
#define IMAGE_H

#include <functional>
#include <optional>
#include <random>

#include "kernel.h"
#include "vec.h"

class Image {
   public:
    int w, h;
    std::vector<ivec4> data;

   private:
    Image& apply_filter(const Kernel& kernel,
                        bool normalise);

   public:
    Image(int w, int h);
    explicit Image(const std::vector<ivec4>& data, int w,
                   int h);
    explicit Image(const std::vector<uvec4>& data, int w,
                   int h);

    const ivec4& get_px(int x, int y) const;
    ivec4& get_px(int x, int y);

    void set_px(int x, int y, const ivec4& v);

    Image duplicate() const;

    Image& posterise(bool ignore_alpha);
    Image& streak(
        const std::vector<int> h_iter,
        const std::vector<int> v_iter,
        const std::function<
            std::optional<int>(int, int, int, int, int)>
            get_streak_idx,
        const std::function<int(const ivec4&)> measure,
        const std::optional<Image>& measure_source =
            std::nullopt);
    Image& streak_down(const std::optional<Image>&
                           measure_source = std::nullopt);
    Image& streak_up(const std::optional<Image>&
                         measure_source = std::nullopt);
    Image& streak_left(const std::optional<Image>&
                           measure_source = std::nullopt);
    Image& streak_right(const std::optional<Image>&
                            measure_source = std::nullopt);

    Image& apply_function(
        const std::function<ivec4(ivec4&)> f);

    Image& add(const Image& other, double other_ratio);

    Image& half_size();
    Image& abs();
    Image& clamp_zero();
    Image& hard_clamp(double max = 255.0);
    Image& smooth_clamp(double half = 127.0,
                        double max = 255.0);
    Image& modulo(int mod);

    Image& scale(double c);
    Image& remove_red();
    Image& remove_green();
    Image& remove_blue();
    Image& black_and_white();

    Image& rgb_to_hsv();
    Image& hsv_to_rgb();

    Image& sobel_horizontal(bool normalise);
    Image& sobel_vertical(bool normalise);
    Image& laplacian3(bool normalise);
    Image& laplacian5(bool normalise);
    Image& box(bool normalise = true);
    Image& gaussian(bool normalise = true);
};

#endif
