#pragma once

#include "renderer.h"
#include "coord.h"
#include "buffer.h"
#include "inputs.h"

#include <variant>

namespace cgull {
    struct char_action { char32_t key; };
    struct special_key_action { key_code key; unsigned int mods; };
    struct click_action { coord click_coord; mouse_button mb; };
    struct resize_action { coord size; };

    using action = std::variant<char_action,
                                special_key_action,
                                click_action,
                                resize_action>;

    struct editor {
        coord window_size;
        renderer* renderer_ptr;
        buffer buf;
        key_map keys;

        editor(coord w_size, key_map km);
        void update(std::vector<action> actions);
        void compile();
    };
}