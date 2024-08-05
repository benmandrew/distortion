#ifndef IMAGE_H
#define IMAGE_H

#include <random>
#include <functional>
#include <optional>

#include "vec.h"

class Image {
  public:
    ImgData data;
    u_int w, h;

    Image(u_int w, u_int h);
    Image(ImgData &data, u_int w, u_int h);

    size_t size();
    Image posterise(bool ignore_alpha);
    Image streak(std::vector<int> h_iter, std::vector<int> v_iter,
                 std::function<std::optional<int>(int, int, int, int, int)> get_streak_idx,
                 std::function<int(const vec4&)> measure,
                 const std::optional<Image> &measure_source = std::nullopt);
    Image streak_down(const std::optional<Image> &measure_source = std::nullopt);
    Image streak_up(const std::optional<Image> &measure_source = std::nullopt);
    Image streak_left(const std::optional<Image> &measure_source = std::nullopt);
    Image streak_right(const std::optional<Image> &measure_source = std::nullopt);

    Image sobel_horizontal() const;
};

#endif
