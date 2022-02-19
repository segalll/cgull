#include "buffer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "nfd.hpp"
#include "utf8.h"

namespace {
constexpr bool is_quote(char32_t character) { return character == U'"' || character == U'\''; }

constexpr bool is_bracket(char32_t character) { return character == U'(' || character == U'{' || character == U'['; }

constexpr bool is_closing_bracket(char32_t character) { return character == U')' || character == U'}' || character == U']'; }

constexpr char32_t corresponding_bracket(char32_t bracket) { return bracket == U'(' ? ')' : bracket + 2; }

constexpr bool is_container(char32_t character) {
    return character == U'(' || character == U'{' || character == U'[' || character == U'"' || character == U'\'';
}

constexpr bool is_closing_container(char32_t character) { return is_quote(character) || is_closing_bracket(character); }

constexpr char32_t corresponding_container(char32_t character) {
    return is_quote(character) ? character : corresponding_bracket(character);
}
} // namespace

namespace cgull {
buffer::buffer() { content = {{}}; }

buffer::buffer(const std::string& filepath) : file_path(filepath) {
    content = {};

    std::ifstream f;
    f.open(filepath);

    std::string line;
    while (std::getline(f, line)) {
        content.push_back(utf8::utf8to32(line));
    }

    f.close();

    if (content.empty()) {
        content.push_back({});
    }
}

void buffer::enter_char(char32_t new_char) {
    if (selection_start == std::nullopt) {
        std::u32string insertion;
        if (is_closing_container(new_char) && content[cursor.row].size() > cursor.col &&
            content[cursor.row][cursor.col] == new_char) {
            insertion = {};
        } else if (is_container(new_char)) {
            insertion = {new_char, corresponding_container(new_char)};
        } else {
            insertion = {new_char};
        }
        content[cursor.row].insert(cursor.col, insertion);
        cursor.col += 1;
    } else {
        if (is_container(new_char)) {
            auto [front, back] = get_selection_ends();
            content[front.row].insert(front.col, {new_char});
            selection_start->col += 1;
            content[back.row].insert(back.col + 1, {corresponding_container(new_char)});
            cursor.col += 1;
        } else {
            delete_selection();
            content[cursor.row].insert(cursor.col, {new_char});
            cursor.col += 1;
        }
    }
}

void buffer::new_line() {
    int indent_pos = content[cursor.row].find_first_not_of(U' ');
    indent_pos = indent_pos == -1 ? content[cursor.row].size() : indent_pos;
    std::u32string indent;
    indent.insert(0, indent_pos, U' ');
    content.insert(content.begin() + cursor.row + 1, indent + content[cursor.row].substr(cursor.col));
    content[cursor.row].erase(content[cursor.row].begin() + cursor.col, content[cursor.row].end());
    if (cursor.col > 0 && content[cursor.row][cursor.col - 1] == U'{' && content[cursor.row + 1][indent_pos] == U'}') {
        content.insert(content.begin() + cursor.row + 1, indent + U"    ");
        cursor.col = content[cursor.row + 1].size();
    } else {
        cursor.col = indent_pos;
    }
    cursor.row += 1;
}

void buffer::indent() {
    int indent_pos = content[cursor.row].find_first_not_of(U' ');
    indent_pos = indent_pos == -1 ? content[cursor.row].size() : indent_pos;
    std::u32string indent = U"";
    int to_insert = indent_pos == 0 ? 4 : 4 - indent_pos % 4;
    indent.insert(0, to_insert, U' ');
    content[cursor.row].insert(cursor.col, indent);
    cursor.col += to_insert;
}

bool buffer::unindent() {
    int indent_pos = content[cursor.row].find_first_not_of(U' ', cursor.col);
    indent_pos = indent_pos == -1 ? content[cursor.row].size() : indent_pos;
    if (indent_pos >= 4 && content[cursor.row].substr(indent_pos - 4, 4) == U"    ") {
        content[cursor.row].erase(indent_pos - 4, 4);
        cursor.col = cursor.col < 4 ? 0 : cursor.col - 4;
        return true;
    }
    return false;
}

void buffer::delete_selection() {
    auto [front, back] = get_selection_ends();
    const int delete_count = back.row - front.row;
    if (delete_count > 0) {
        content[front.row].erase(front.col);
        content[back.row].erase(0, back.col);
        content[front.row].insert(content[front.row].size(), content[back.row]);
        content.erase(content.begin() + front.row + 1, content.begin() + back.row + 1);
    } else {
        content[front.row].erase(front.col, back.col - front.col);
    }
    cursor = front;
    selection_start = std::nullopt;
}

void buffer::backspace() {
    if (selection_start == std::nullopt) {
        if (unindent()) {
            return;
        }
        if (cursor.col > 0) {
            cursor.col -= 1;
            if (is_container(content[cursor.row][cursor.col]) &&
                content[cursor.row][cursor.col + 1] == corresponding_container(content[cursor.row][cursor.col])) {
                content[cursor.row].erase(cursor.col, 2);
            } else {
                content[cursor.row].erase(content[cursor.row].begin() + cursor.col);
            }
        } else if (cursor.row > 0) {
            cursor.col = content[cursor.row - 1].size();
            content[cursor.row - 1] += content[cursor.row];
            content.erase(content.begin() + cursor.row);
            cursor.row -= 1;
        }
    } else {
        delete_selection();
    }
}

void buffer::cursor_left() {
    if (selection_start == std::nullopt) {
        if (cursor.col == 0) {
            if (cursor.row != 0) {
                cursor.row = cursor.row - 1 > 0 ? cursor.row - 1 : 0;
                cursor.col = content[cursor.row].size();
            }
        } else {
            cursor.col -= 1;
        }
    } else {
        auto [front, _] = get_selection_ends();
        cursor = front;
        selection_start = std::nullopt;
    }
}

void buffer::cursor_right() {
    if (selection_start == std::nullopt) {
        if (cursor.col >= content[cursor.row].size()) {
            if (cursor.row < content.size() - 1) {
                cursor.row += 1;
                cursor.col = 0;
            }
        } else {
            cursor.col += 1;
        }
    } else {
        auto [_, back] = get_selection_ends();
        cursor = back;
        selection_start = std::nullopt;
    }
}

void buffer::cursor_up() {
    if (selection_start != std::nullopt) {
        auto [front, _] = get_selection_ends();
        cursor = front;
        selection_start = std::nullopt;
    }
    if (cursor.row == 0) {
        cursor.col = 0;
    } else {
        cursor.row -= 1;
        cursor.col = content[cursor.row].size() < cursor.col ? content[cursor.row].size() : cursor.col;
    }
}

void buffer::cursor_down() {
    if (selection_start != std::nullopt) {
        auto [_, back] = get_selection_ends();
        cursor = back;
        selection_start = std::nullopt;
    }
    if (cursor.row >= content.size() - 1) {
        cursor.col = content.back().size();
    } else {
        cursor.row += 1;
        cursor.col = content[cursor.row].size() < cursor.col ? content[cursor.row].size() : cursor.col;
    }
}

void buffer::save() {
    std::ofstream f;
    if (file_path == std::nullopt) {
        NFD::Guard nfdGuard;
        NFD::UniquePath outPath;
        nfdresult_t result = NFD::SaveDialog(outPath, nullptr, 0);
        if (result != NFD_OKAY) {
            return;
        }
        file_path = outPath.get();
    }
    f.open(file_path.value());
    std::string out;
    for (const line& l : content) {
        out += utf8::utf32to8(l) + "\n";
    }
    f << out;

    f.close();
}

void buffer::cursor_click(coord click) {
    cursor = click;
    selection_start = std::nullopt;
    if (click.row > content.size()) {
        cursor.row = content.size() - 1;
        cursor.col = content.back().size();
    }
}

bool buffer::cursor_move(coord pos) {
    coord old_cursor = cursor;
    cursor = pos;
    if (old_cursor != cursor) {
        if (selection_start == std::nullopt) {
            selection_start = old_cursor;
        }
        return true;
    }
    return false;
}
} // namespace cgull