#pragma once

#include "coord.h"

#include <vector>
#include <optional>

namespace cgull {
    using line = std::vector<unsigned int>;
    using text = std::vector<line>;

    struct snapshot {
        text content;
        coord cursor;
    };

    struct buffer {
        // file from;
        text content;
        coord cursor;
        coord scroll;
        std::optional<coord> selection_start;
        std::vector<snapshot> history;
        std::optional<std::size_t> history_pos;

        buffer();
        void backspace();
        void enter_char(unsigned int new_char);
        void new_line();
    };
}