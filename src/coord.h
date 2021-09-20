#pragma once

namespace cgull {
    using index = int;

    struct coord {
        index row = {};
        index col = {};
    };

    coord coord_from_pixel(int x, int y);
}