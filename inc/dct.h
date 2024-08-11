#ifndef DCT_H
#define DCT_H

#include "image.h"

class Dct {
   public:
    int w, h;

   private:
    std::vector<dvec4> data;

    const dvec4& get_px(int bi, int bj, int i, int j) const;
    dvec4& get_px(int bi, int bj, int i, int j);
    int block_idx(int bi, int bj, int i, int j) const;
    void encode_block(std::vector<dvec4>& decode, int bi,
                      int bj);
    void decode_block(std::vector<ivec4>& out, int bi,
                      int bj) const;

   public:
    Dct(const Image& image);
    Dct(const std::vector<ivec4>& data, int w, int h);

    Image dump_image() const;
    Image to_image_decode() const;
};

#endif
