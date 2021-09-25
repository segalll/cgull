#pragma once

#include "coord.h"
#include "buffer.h"
#include "inputs.h"

#include <variant>

namespace cgull {
    struct char_action { key_code key; };
    struct special_key_action { key_code key; unsigned int mods; };
    struct click_action { coord click_coord; mouse_button mb; };
    struct resize_action { coord size; };

    using action = std::variant<char_action,
                                special_key_action,
                                click_action,
                                resize_action>;

    struct editor {
        coord window_size;
        key_map keys;
        buffer buf;

        editor(key_map km);
        void update(action a);
    };
}