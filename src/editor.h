#pragma once

#include "buffer.h"
#include "coord.h"
#include "inputs.h"
#include "renderer.h"

#include <future>
#include <variant>

namespace cgull {
struct char_action {
    char32_t key;
};
struct special_key_action {
    key_code key;
    unsigned int mods;
};
struct click_action {
    coord click_coord;
    mouse_button mb;
    bool pressed;
};
struct resize_action {
    coord size;
};
struct scroll_action {
    float x_offset;
    float y_offset;
};
struct mouse_move_action {
    coord pos;
};

using action =
    std::variant<char_action, special_key_action, click_action, resize_action, scroll_action, mouse_move_action>;

struct editor {
    coord window_size;
    renderer* renderer_ptr;
    buffer buf;
    key_map keys;
    bool mouse_down = false;
    std::future<void> f;

    editor(coord w_size, key_map km);
    void update(std::vector<action> actions);
    void compile();
    void open();
};
} // namespace cgull