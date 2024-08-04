#ifndef IMG_H
#define IMG_H

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
                 std::function<std::optional<int>(int, int, int, int, int)> get_streak_idx);
    Image streak_down();
    Image streak_up();
    Image streak_left();
    Image streak_right();
};

#endif
