#pragma once

#include "coord.h"

#include <vector>
#include <optional>
#include <fstream>

namespace cgull {
    using line = std::u32string;
    using text = std::vector<line>;

    struct snapshot {
        text content;
        coord cursor;
    };

    struct file {
        std::fstream stream;
        std::string name;
    };

    struct buffer {
        file from;
        text content;
        coord cursor;
        coord scroll;
        std::optional<coord> selection_start;
        std::vector<snapshot> history;
        std::optional<std::size_t> history_pos;

        buffer();
        void enter_char(char32_t new_char);
        void new_line();
        void tab();
        void backspace();
        void cursor_left();
        void cursor_right();
        void cursor_up();
        void cursor_down();
        void save();
    };
}