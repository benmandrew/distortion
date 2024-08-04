#ifndef VEC_H
#define VEC_H

#include "lodepng.h"

template <typename T> struct vec4_T {
    T r, g, b, a;

    static constexpr const vec4_T zero = {
        .r = static_cast<T>(0),
        .g = static_cast<T>(0),
        .b = static_cast<T>(0),
        .a = static_cast<T>(0),
    };

    vec4_T add(vec4_T &x);
    vec4_T saturating_sub(vec4_T &x);
    vec4_T sub(vec4_T &x);
    double luminance();

    vec4_T v_abs();
};

template <typename T> vec4_T<T> vec4_T<T>::add(vec4_T &x) {
    return vec4_T{
        .r = static_cast<u_char>(x.r + r),
        .g = static_cast<u_char>(x.g + g),
        .b = static_cast<u_char>(x.b + b),
        .a = static_cast<u_char>(x.a + a)
    };
}

inline u_char saturating_sub(u_char x, u_char y) {
    if (y > x) {
        return 0;
    }
    return x - y;
}

template <typename T> vec4_T<T> vec4_T<T>::saturating_sub(vec4_T &x) {
    return vec4_T{
        .r = saturating_sub(r, x.r),
        .g = saturating_sub(g, x.g),
        .b = saturating_sub(b, x.b),
        .a = saturating_sub(a, x.a)
    };
}

template <typename T> vec4_T<T> vec4_T<T>::sub(vec4_T &x) {
    return vec4_T{
        .r = r - x.r,
        .g = g - x.g,
        .b = b - x.b,
        .a = a - x.a
    };
}

template <typename T> double vec4_T<T>::luminance() {
    return 0.2126 * static_cast<double>(r) + 0.7152 * static_cast<double>
           (g) + 0.0722 * static_cast<double>(b);
}

template <typename T> vec4_T<T> vec4_T<T>::v_abs() {
    return vec4_T{
        .r = abs(r),
        .g = abs(g),
        .b = abs(b),
        .a = abs(a),
    };
}

using vec4 = vec4_T<u_char>;

using ImgData = std::vector<vec4>;

inline vec4_T<int> vec4_to_ints(vec4 &v) {
    return vec4_T<int> {
        .r = static_cast<int>(v.r),
        .g = static_cast<int>(v.g),
        .b = static_cast<int>(v.b),
        .a = static_cast<int>(v.a),
    };
}

inline vec4 ints_to_vec4(vec4_T<int> &v) {
    return vec4 {
        .r = static_cast<u_char>(v.r),
        .g = static_cast<u_char>(v.g),
        .b = static_cast<u_char>(v.b),
        .a = static_cast<u_char>(v.a),
    };
}

#endif
