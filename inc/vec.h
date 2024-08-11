#ifndef VEC_H
#define VEC_H

#include "lodepng.h"

template <typename T>
struct vec4 {
    T r, g, b, a;

    static constexpr const vec4 zero = {
        .r = static_cast<T>(0),
        .g = static_cast<T>(0),
        .b = static_cast<T>(0),
        .a = static_cast<T>(255),
    };

    template <typename U>
    static vec4 create(U r, U g, U b, U a);

    void print() const;

    vec4 add(const vec4& x) const;
    vec4 v_saturating_sub(const vec4& x) const;
    vec4 sub(const vec4& x) const;
    template <typename U>
    vec4 scale(const U c) const;
    double luminance() const;

    vec4 abs() const;
    vec4 min_zero() const;
    vec4 hard_clamp(double max = 255.0) const;
    vec4 smooth_clamp(double half = 127.0,
                      double max = 255.0) const;
    vec4 modulo(T mod) const;
};

template <typename T>
template <typename U>
vec4<T> vec4<T>::create(U r, U g, U b, U a) {
    return vec4{
        .r = static_cast<T>(r),
        .g = static_cast<T>(g),
        .b = static_cast<T>(b),
        .a = static_cast<T>(a),
    };
}

#include <iomanip>
#include <iostream>

template <typename T>
void print_image(const std::vector<vec4<T>>& data, int w,
                 int h) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int idx = j * w + i;
            std::cout << std::setfill(' ') << std::setw(4)
                      << +data[idx].r;
        }
        std::cout << std::endl;
    }
}

template <typename T>
vec4<T> vec4<T>::add(const vec4<T>& x) const {
    return vec4{.r = static_cast<T>(x.r + r),
                .g = static_cast<T>(x.g + g),
                .b = static_cast<T>(x.b + b),
                .a = static_cast<T>(x.a + a)};
}

template <typename T>
inline T saturating_sub(const T x, const T y) {
    if (y > x) {
        return 0;
    }
    return x - y;
}

template <typename T>
vec4<T> vec4<T>::v_saturating_sub(const vec4& x) const {
    return vec4{.r = saturating_sub(r, x.r),
                .g = saturating_sub(g, x.g),
                .b = saturating_sub(b, x.b),
                .a = saturating_sub(a, x.a)};
}

template <typename T>
vec4<T> vec4<T>::sub(const vec4& x) const {
    return vec4{.r = r - x.r,
                .g = g - x.g,
                .b = b - x.b,
                .a = a - x.a};
}

template <typename T>
template <typename U>
vec4<T> vec4<T>::scale(const U c) const {
    return vec4{
        .r = static_cast<T>(c * static_cast<U>(r)),
        .g = static_cast<T>(c * static_cast<U>(g)),
        .b = static_cast<T>(c * static_cast<U>(b)),
        // .a = static_cast<T>(c * static_cast<U>(a)),
        .a = static_cast<T>(255),
    };
}

template <typename T>
double vec4<T>::luminance() const {
    return 0.2126 * static_cast<double>(r) +
           0.7152 * static_cast<double>(g) +
           0.0722 * static_cast<double>(b);
}

template <typename T>
vec4<T> vec4<T>::abs() const {
    return vec4{
        .r = std::abs(r),
        .g = std::abs(g),
        .b = std::abs(b),
        .a = std::abs(a),
    };
}

template <typename T>
vec4<T> vec4<T>::min_zero() const {
    return vec4{
        .r = std::max(r, static_cast<T>(0)),
        .g = std::max(g, static_cast<T>(0)),
        .b = std::max(b, static_cast<T>(0)),
        .a = std::max(a, static_cast<T>(0)),
    };
}

template <typename T>
vec4<T> vec4<T>::hard_clamp(double max) const {
    auto f = [max](T v) {
        return std::max(std::min(v, static_cast<T>(max)),
                        static_cast<T>(0));
    };
    return vec4{
        .r = f(r),
        .g = f(g),
        .b = f(b),
        .a = f(a),
    };
}

template <typename T>
vec4<T> vec4<T>::smooth_clamp(double half,
                              double max) const {
    double rd = static_cast<double>(r);
    double gd = static_cast<double>(g);
    double bd = static_cast<double>(b);
    return vec4{
        .r = static_cast<T>(max * rd / (rd + half)),
        .g = static_cast<T>(max * gd / (gd + half)),
        .b = static_cast<T>(max * bd / (bd + half)),
        .a = 255,
    };
}

template <typename T>
vec4<T> vec4<T>::modulo(T mod) const {
    auto f = [mod](T v) {
        while (v < static_cast<T>(0)) {
            v += mod;
        }
        return v % mod;
    };
    return vec4{
        .r = f(r),
        .g = f(g),
        .b = f(b),
        .a = 255,
    };
}

using uvec4 = vec4<u_char>;
using ivec4 = vec4<int>;
using dvec4 = vec4<double>;

inline ivec4 uvec4_to_ivec4(const uvec4& v) {
    return ivec4{
        .r = static_cast<int>(v.r),
        .g = static_cast<int>(v.g),
        .b = static_cast<int>(v.b),
        .a = static_cast<int>(v.a),
    };
}

inline uvec4 ivec4_to_uvec4(const ivec4& v) {
    return vec4{
        .r = static_cast<u_char>(v.r),
        .g = static_cast<u_char>(v.g),
        .b = static_cast<u_char>(v.b),
        .a = static_cast<u_char>(v.a),
    };
}

inline ivec4 dvec4_to_ivec4(const dvec4& v) {
    return ivec4{
        .r = static_cast<int>(v.r),
        .g = static_cast<int>(v.g),
        .b = static_cast<int>(v.b),
        .a = static_cast<int>(v.a),
    };
}

inline dvec4 ivec4_to_dvec4(const ivec4& v) {
    return vec4{
        .r = static_cast<double>(v.r),
        .g = static_cast<double>(v.g),
        .b = static_cast<double>(v.b),
        .a = static_cast<double>(v.a),
    };
}

#endif
