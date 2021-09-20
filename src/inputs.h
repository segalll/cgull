#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace cgull {
    using key_code = unsigned int;
    using key_seq = std::vector<key_code>;
    using key_map = std::unordered_map<key_seq, std::string>;

    enum class mouse_button {
        left,
        right
    };

    enum class key_mods {
        shift,
        ctrl,
        alt,
        super,
        caps_lock,
        num_lock,
        none
    };
}