#ifndef IMAGE_H
#define IMAGE_H

#include <functional>
#include <optional>
#include <random>

#include "vec.h"

class Image {
   public:
    ImgData data;
    u_int w, h;

    Image(u_int w, u_int h);
    Image(const ImgData &data, u_int w, u_int h);

    Image posterise(bool ignore_alpha) const;
    Image streak(
        const std::vector<int> h_iter, const std::vector<int> v_iter,
        const std::function<std::optional<int>(int, int, int, int, int)>
            get_streak_idx,
        const std::function<int(const vec4 &)> measure,
        const std::optional<Image> &measure_source = std::nullopt) const;
    Image streak_down(
        const std::optional<Image> &measure_source = std::nullopt) const;
    Image streak_up(
        const std::optional<Image> &measure_source = std::nullopt) const;
    Image streak_left(
        const std::optional<Image> &measure_source = std::nullopt) const;
    Image streak_right(
        const std::optional<Image> &measure_source = std::nullopt) const;

    Image sobel_horizontal() const;
};

#endif
