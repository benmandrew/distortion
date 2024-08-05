#include "sobel.h"

Image Sobel::to_image() {
    ImgData out(w * h);
    for (int i = 0; i < data.size(); i++) {
        vec4_T<int> v = data[i].scale(10.0).v_abs();
        out[i] = ints_to_vec4(v);
    }
    return Image(out, w, h);
}

Sobel Sobel::new_horizontal(const Image &image) {
    Sobel out;
    out.w = image.w;
    out.h = image.h;
    out.data = horizontal(image);
    return out;
}

Sobel Sobel::new_vertical(const Image &image) {
    Sobel out;
    out.w = image.w;
    out.h = image.h;
    out.data = vertical(image);
    return out;
}

std::vector<vec4_T<int>> Sobel::horizontal(const Image &image) {
    std::vector<vec4_T<int>> inter(image.w * image.h);
    std::vector<vec4_T<int>> data(image.w * image.h);
    for (int i = 0; i < image.data.size(); i++) {
        inter[i] = vec4_to_ints(image.data[i]);
    }
    for (int j = 0; j < image.h; j++) {
        for (int i = 0; i < image.w; i++) {
            int idx = j * image.w + i;
            if (i == 0 or i == image.w - 1 or j == 0 or j == image.h - 1) {
                data[idx] = vec4_T<int>::zero;
            } else {
                vec4_T<int> pos =
                    inter[idx - image.w + 1].add(
                        inter[idx + 1].scale(2.0).add(
                            inter[idx + image.w + 1]));
                vec4_T<int> neg =
                    inter[idx - image.w - 1].add(
                        inter[idx - 1].scale(2.0).add(
                            inter[idx + image.w - 1]));
                data[idx] = pos.sub(neg).scale(0.125);
            }
            data[idx].a = 255;
        }
    }
    return data;
}

std::vector<vec4_T<int>> Sobel::vertical(const Image &image) {
    std::vector<vec4_T<int>> inter(image.w * image.h);
    std::vector<vec4_T<int>> data(image.w * image.h);
    for (int i = 0; i < image.data.size(); i++) {
        inter[i] = vec4_to_ints(image.data[i]);
    }
    for (int j = 0; j < image.h; j++) {
        for (int i = 0; i < image.w; i++) {
            int idx = j * image.w + i;
            if (i == 0 or i == image.w - 1 or j == 0 or j == image.h - 1) {
                data[idx] = vec4_T<int>::zero;
            } else {
                vec4_T<int> pos =
                    inter[idx - image.w - 1].add(
                        inter[idx - image.w].scale(2.0).add(
                            inter[idx - image.w + 1]));
                vec4_T<int> neg =
                    inter[idx + image.w - 1].add(
                        inter[idx + image.w].scale(2.0).add(
                            inter[idx + image.w + 1]));
                data[idx] = pos.sub(neg).scale(0.125);
            }
            data[idx].a = 255;
        }
    }
    return data;
}
