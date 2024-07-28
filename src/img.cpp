#include "defs.h"

#include <iostream>

#define POST_LEVELS 4
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
