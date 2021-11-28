#pragma once

#include "coord.h"

#include <fstream>
#include <optional>
#include <vector>

namespace cgull {
using line = std::u32string;
using text = std::vector<line>;

struct snapshot {
    text content;
    coord cursor;
};

struct buffer {
    text content;
    coord cursor;
    coord scroll;
    std::optional<coord> selection_start;
    std::vector<snapshot> history;
    std::optional<std::size_t> history_pos;
    std::optional<std::string> file_path;

    buffer();
    buffer(const std::string& filepath);
    void enter_char(char32_t new_char);
    void new_line();
    void indent();
    bool unindent();
    void backspace();
    void cursor_left();
    void cursor_right();
    void cursor_up();
    void cursor_down();
    void save();
};
} // namespace cgull