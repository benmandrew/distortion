#include <queue>
#include <set>

#include "image.h"

struct Pixel {
   public:
    int x, y;
    double lum;

    Pixel(int x, int y, double lum)
        : x(x), y(y), lum(lum) {}
};

auto compare_lum = [](const Pixel& p1, const Pixel& p2) {
    return p1.lum < p2.lum;
};

auto compare_loc = [](const Pixel& p1, const Pixel& p2) {
    if (p1.x < p2.x) {
        return true;
    } else if (p1.x > p2.x) {
        return false;
    }
    return p1.y < p2.y;
};

Pixel create_px(const Image& image, int x, int y) {
    double lum = image.get_px(x, y).luminance();
    return Pixel(x, y, lum);
}

typedef std::set<Pixel, decltype(compare_loc)> PixelSet;
typedef std::priority_queue<Pixel, std::vector<Pixel>,
                            decltype(compare_lum)>
    PixelQueue;

std::vector<Pixel> new_neighbours(const Image& image,
                                  PixelSet& visited, int x,
                                  int y) {
    std::vector<Pixel> v;
    if (x > 0) {
        Pixel px = create_px(image, x - 1, y);
        if (!visited.contains(px)) {
            v.push_back(px);
        }
    }
    if (x + 1 < image.w) {
        Pixel px = create_px(image, x + 1, y);
        if (!visited.contains(px)) {
            v.push_back(px);
        }
    }
    if (y > 0) {
        Pixel px = create_px(image, x, y - 1);
        if (!visited.contains(px)) {
            v.push_back(px);
        }
    }
    if (y + 1 < image.h) {
        Pixel px = create_px(image, x, y + 1);
        if (!visited.contains(px)) {
            v.push_back(px);
        }
    }
    return v;
}

ivec4 colour(const Image& image, int i) {
    constexpr double colour_max =
        static_cast<double>(0xFFFFFF);
    int max = image.w * image.h;
    int colour = static_cast<int>(
        (static_cast<double>(i)) *
        (colour_max / static_cast<double>(max)));
    return ivec4((colour & 0xFF0000) >> 16,
                 (colour & 0x00FF00) >> 8,
                 colour & 0x0000FF, 255);
    // constexpr double colour_max =
    // static_cast<double>(0xFF); int max = image.w *
    // image.h; int colour = static_cast<int>(
    //     (static_cast<double>(colour_max - i)) *
    //     (colour_max / static_cast<double>(max)));
    // return ivec4(colour, colour, colour, 255);
}

Image sort(const Image& image, int x, int y) {
    PixelQueue frontier;
    PixelSet visited;
    Image new_image(image.w, image.h);
    Pixel curr = create_px(image, x, y);
    visited.insert(curr);
    for (const Pixel& px :
         new_neighbours(image, visited, curr.x, curr.y)) {
        frontier.push(px);
        visited.insert(px);
    }
    int i = 0;
    while (!frontier.empty()) {
        curr = frontier.top();
        frontier.pop();
        ivec4 v = colour(image, i);
        i++;
        new_image.set_px(curr.x, curr.y, v);
        visited.insert(curr);
        for (const Pixel& px : new_neighbours(
                 image, visited, curr.x, curr.y)) {
            frontier.push(px);
            visited.insert(px);
        }
    }
    return new_image;
}
