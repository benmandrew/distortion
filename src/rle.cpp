#include "rle.h"

void Rle::encode(const std::vector<ivec4>& data) {
    const auto& v = data[0];
    int runlength = 1;
    int i = 1;
    int len = data.size();
    std::vector<int> lengths;
    std::vector<ivec4> colours;
    while (i < len) {
        const auto& v_tmp = data[i];
        if (v.r != v_tmp.r || v.g != v_tmp.g ||
            v.b != v_tmp.b || v.a != v_tmp.a) {
            lengths.push_back(runlength);
            colours.push_back(v);
            runlength = 1;
        } else {
            runlength++;
        }
        i++;
    }
    lengths.push_back(runlength);
    colours.push_back(v);
}

Rle::Rle(const Image& image) : w{image.w}, h{image.h} {
    encode(image.data);
}

Image Rle::to_image() const {
    std::vector<ivec4> out;
    int rl_len = lengths.size();
    for (int i = 0; i < rl_len; i++) {
        int runlength = lengths[i];
        for (int j = 0; j < runlength; j++) {
            out.push_back(colours[i]);
        }
    }
    return Image(out, w, h);
}

void Rle::add_noise(double stddev = 1.0) {
    std::default_random_engine gen;
    std::normal_distribution<double> dist(0.0, stddev);
    int rl_len = lengths.size();
    int target_len = w * h;
    int total_len = 0;
    for (int i = 0; i < rl_len; i++) {
        int runlength = static_cast<int>(lengths[i]);
        int offset = static_cast<int>(dist(gen));
        if (runlength < -offset) {
            lengths[i] = 0;
        } else {
            lengths[i] =
                static_cast<int>(runlength + offset);
        }
        total_len += lengths[i];
        if (total_len > target_len) {
            lengths[i] -= total_len - target_len;
            break;
        }
    }
    if (total_len < target_len) {
        lengths[lengths.size() - 1] +=
            target_len - total_len;
    }
}

void Rle::add_noise_rows(double stddev = 1.0) {
    std::default_random_engine gen;
    std::normal_distribution<double> dist(0.0, stddev);
    int n_prev_runs = 0;
    for (int j = 0; j < h; j++) {
        int runs_in_row = 0;
        int row_len = 0;
        while (row_len < w) {
            row_len += lengths[n_prev_runs + runs_in_row];
            runs_in_row++;
        }
        row_len = 0;
        for (int i = 0; i < runs_in_row; i++) {
            int idx = n_prev_runs + i;
            int runlength = static_cast<int>(lengths[idx]);
            int offset = static_cast<int>(dist(gen));
            if (runlength < -offset) {
                lengths[idx] = 0;
            } else {
                lengths[idx] = runlength + offset;
            }
            row_len += lengths[idx];
            if (row_len > w) {
                row_len -= lengths[idx];
                lengths[idx] = 0;
            }
        }
        if (row_len <= w) {
            lengths[n_prev_runs + runs_in_row - 1] +=
                w - row_len;
        }
        n_prev_runs += runs_in_row;
    }
}
