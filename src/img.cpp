#include "defs.h"

#include <random>

#define POST_LEVELS 8
#define POST_COEFF (u_char)(256 / POST_LEVELS) 

class Image {
public:
   ImgData data;
   u_int w, h;

   Image(u_int w, u_int h);
   Image(ImgData &data, u_int w, u_int h);

   size_t size();
   Image posterise(bool ignore_alpha);
};

Image::Image(u_int w, u_int h) {
   this->data = ImgData(w * h * 4);
   this->w = w;
   this->h = h;
}

Image::Image(ImgData &data, u_int w, u_int h) {
   this->data = data;
   this->w = w;
   this->h = h;
}

size_t Image::size() {
   return w * h;
}

u_char posterise_value(u_char v) {
   if (v == 255) {
      return v;
   }
   return (v / POST_COEFF) * POST_COEFF;
}

Image Image::posterise(bool ignore_alpha = true) {
   Image out(w, h);
   for (int i = 0; i < data.size(); i++) {
      if (ignore_alpha && i % 4 == 3) {
         out.data[i] = data[i];
      } else {
         out.data[i] = posterise_value(data[i]);
      }
   }
   return out;
}

// Run-length encoded image
class Rle {
public:
   std::vector<size_t> data;
   u_int w, h;

   Rle(Image &image);

   Image to_image();

   void add_noise(double stddev);
   void add_noise_rows(double stddev);
};

std::vector<size_t> encode(ImgData &data) {
   u_char r = data[0];
   u_char g = data[1];
   u_char b = data[2];
   u_char a = data[3];
   size_t runlength = 1;
   size_t i = 1;
   size_t len = data.size() / 4;
   std::vector<size_t> out;
   while (i < len) {
      if (r != data[i * 4]
         || g != data[i * 4 + 1]
         || b != data[i * 4 + 2]
         || a != data[i * 4 + 3]) {
         out.insert(out.end(), {runlength, r, g, b, a});
         runlength = 1;
         r = data[i * 4];
         g = data[i * 4 + 1];
         b = data[i * 4 + 2];
         a = data[i * 4 + 3];
      } else {
         runlength++;
      }
      i++;
   }
   out.insert(out.end(), {runlength, r, g, b, a});
   return out;
}

ImgData decode(std::vector<size_t> &data, u_int w, u_int h) {
   ImgData out;
   size_t rl_len = data.size() / 5;
   for (int i = 0; i < rl_len; i++) {
      size_t runlength = data[i * 5];
      for (int j = 0; j < runlength; j++) {
         u_char r = data[i * 5 + 1];
         u_char g = data[i * 5 + 2];
         u_char b = data[i * 5 + 3];
         u_char a = data[i * 5 + 4];
         out.insert(out.end(), {r, g, b, a});
      }
   }
   return out;
}

Rle::Rle(Image &image) {
   this->data = encode(image.data);
   this->w = image.w;
   this->h = image.h;
}

Image Rle::to_image() {
   ImgData d = decode(data, w, h);
   return Image(d, w, h);
}

void Rle::add_noise(double stddev = 1.0) {
   std::default_random_engine gen;
   std::normal_distribution<double> dist(0.0, stddev);
   size_t rl_len = data.size() / 5;
   size_t target_len = w * h;
   size_t total_len = 0;
   for (int i = 0; i < rl_len; i++) {
      int runlength = int(data[i * 5]);
      int offset = int(dist(gen));
      if (runlength < -offset) {
         data[i * 5] = 0;
      } else {
         data[i * 5] = size_t(runlength + offset);
      }
      total_len += data[i * 5];
      if (total_len > target_len) {
         data[i * 5] -= total_len - target_len;
         break;
      }
   }
   if (total_len < target_len) {
      data[data.size() - 5] += target_len - total_len;
   }
}

void Rle::add_noise_rows(double stddev = 1.0) {
   std::default_random_engine gen;
   std::normal_distribution<double> dist(0.0, stddev);
   size_t n_prev_runs = 0;
   for (int j = 0; j < h; j++) {
      size_t runs_in_row = 0;
      size_t row_len = 0;
      while (row_len < w) {
         row_len += data[(n_prev_runs + runs_in_row) * 5];
         runs_in_row++;
      }
      row_len = 0;
      for (int i = 0; i < runs_in_row; i++) {
         size_t idx = (n_prev_runs + i) * 5;
         int runlength = int(data[idx]);
         int offset = int(dist(gen));
         if (runlength < -offset) {
            data[idx] = 0;
         } else {
            data[idx] = size_t(runlength + offset);
         }
         row_len += data[idx];
         if (row_len > w) {
            row_len -= data[idx];
            data[idx] = 0;
         }
      }
      if (row_len <= w) {
         data[(n_prev_runs + runs_in_row) * 5 - 5] += w - row_len;
      }
      n_prev_runs += runs_in_row;
   }
}
