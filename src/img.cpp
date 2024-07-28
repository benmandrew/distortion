#include "defs.h"

#include <iostream>

#define POST_LEVELS 4
#define POST_COEFF (u_char)(256 / POST_LEVELS) 

u_char posterise_value(u_char v) {
   if (v == 255) {
      return v;
   }
   return (v / POST_COEFF) * POST_COEFF;
}

Image posterise(Image &v, bool ignore_alpha) {
   Image out(v.size());
   for (int i = 0; i < v.size(); i++) {
      if (ignore_alpha && i % 4 == 3) {
         out[i] = v[i];
      } else {
         out[i] = posterise_value(v[i]);
      }
   }
   return out;
}
