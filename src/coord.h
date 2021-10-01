#pragma once

namespace cgull {
    using index = int;

    struct coord {
        index row = {};
        index col = {};
    };

    inline bool operator==(const coord& a, const coord& b) {
        return a.row == b.row && a.col == b.col;
    }

    coord coord_from_pixel(int x, int y);
}