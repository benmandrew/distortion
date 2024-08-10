#include "dct.h"

static constexpr int B_SIZE = 4;

Dct::Dct(const Image& image)
    : data{std::vector<dvec4>(image.data.size())}, w{image.w}, h{image.h} {
    int bw = w / B_SIZE;
    int bh = h / B_SIZE;
    auto source = std::vector<dvec4>(data.size());
    for (int i = 0; i < data.size(); i++) {
        source[i] = ivec4_to_dvec4(image.data[i]);
    }
    for (int j = 0; j < bh; j++) {
        for (int i = 0; i < bw; i++) {
            encode_block(source, i, j);
        }
    }
}

Image Dct::to_image() const {
    auto out = std::vector<ivec4>(data.size());
    for (int i = 0; i < data.size(); i++) {
        out[i] = dvec4_to_ivec4(data[i]);
    }
    return Image(out, w, h);
}

Image Dct::to_image_decode() const {
    int bw = w / B_SIZE;
    int bh = h / B_SIZE;
    auto out = std::vector<ivec4>(data.size());
    for (int j = 0; j < bh; j++) {
        for (int i = 0; i < bw; i++) {
            decode_block(out, i, j);
        }
    }
    return Image(out, w, h);
}

const dvec4& Dct::get_px(int bi, int bj, int i, int j) const {
    return data[(bj * B_SIZE + j) * w + bi * B_SIZE + j];
}

dvec4& Dct::get_px(int bi, int bj, int i, int j) {
    return const_cast<dvec4&>(
        const_cast<const Dct*>(this)->get_px(bi, bj, i, j));
}

static constexpr double SQRT2O2 = 1.414213 * 0.5;
static constexpr double PI = 3.141592;
static constexpr double INV16 = 1.0 / 16.0;

static constexpr double alpha(int i) {
    if (i == 0) {
        return SQRT2O2 * 0.5;
    }
    return 0.5;
}

static const std::array<double, B_SIZE * B_SIZE> gen_cosine() {
    auto a = std::array<double, B_SIZE * B_SIZE>();
    for (int j = 0; j < B_SIZE; j++) {
        for (int i = 0; i < B_SIZE; i++) {
            a[j * B_SIZE + i] = std::cos(PI * j * (2 * i + 1) * INV16);
        }
    }
    return a;
}

static const std::array<double, B_SIZE * B_SIZE> gen_inv_cosine() {
    auto a = std::array<double, B_SIZE * B_SIZE>();
    for (int j = 0; j < B_SIZE; j++) {
        for (int i = 0; i < B_SIZE; i++) {
            a[j * B_SIZE + i] =
                alpha(i) * alpha(j) * std::cos(PI * j * (2 * i + 1) * INV16);
        }
    }
    return a;
}

static const auto cosine = gen_cosine();
static const auto inv_cosine = gen_inv_cosine();

int Dct::block_idx(int bi, int bj, int i, int j) const {
    return (bj * B_SIZE + j) * w + bi * B_SIZE + i;
}

void Dct::encode_block(std::vector<dvec4>& source, int bi, int bj) {
    for (int v = 0; v < B_SIZE; v++) {
        for (int u = 0; u < B_SIZE; u++) {
            int uv = block_idx(bi, bj, u, v);
            data[uv] = dvec4::zero;
            for (int y = 0; y < B_SIZE; y++) {
                for (int x = 0; x < B_SIZE; x++) {
                    int xy = block_idx(bi, bj, x, y);
                    double coeff =
                        cosine[u * B_SIZE + x] * cosine[v * B_SIZE + y];
                    data[uv] = data[uv].add(source[xy].scale(coeff));
                }
            }
            data[uv] = data[uv].scale(alpha(u) * alpha(v));
            data[uv].a = 255;
        }
    }
}

void Dct::decode_block(std::vector<ivec4>& out, int bi, int bj) const {
    for (int y = 0; y < B_SIZE; y++) {
        for (int x = 0; x < B_SIZE; x++) {
            int xy = block_idx(bi, bj, x, y);
            auto out_vec = dvec4::zero;
            for (int v = 0; v < B_SIZE; v++) {
                for (int u = 0; u < B_SIZE; u++) {
                    int uv = block_idx(bi, bj, u, v);
                    double coeff =
                        cosine[u * B_SIZE + x] * cosine[v * B_SIZE + y];
                    out_vec = out_vec.add(data[uv].scale(coeff));
                }
            }
            out_vec.a = 255;
            out[xy] = dvec4_to_ivec4(out_vec);
        }
    }
}
