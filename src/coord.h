#pragma once

namespace cgull {
using index = unsigned int;

struct coord {
    index row = {};
    index col = {};
};

inline bool operator==(const coord& a, const coord& b) {
    return a.row == b.row && a.col == b.col;
}

coord coord_from_pixel(unsigned int x, unsigned int y);
} // namespace cgull