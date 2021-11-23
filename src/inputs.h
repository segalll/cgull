#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <initializer_list>

namespace cgull {
    using key_code = unsigned int;

    enum class key_mods {
        none = 0x00,
        shift = 0x01,
        ctrl = 0x02,
        alt = 0x04,
        super = 0x08,
        caps_lock = 0x10,
        num_lock = 0x20,
        #ifdef __APPLE__
        platform = 0x08 // main platform modifier is command on macOS
        #else
        platform = 0x02 // ctrl on other platforms
        #endif
    };

    struct key_combination {
        key_code key;
        unsigned int mods;

        key_combination(key_code k, key_mods m) {
            if (k >= 97 && k <= 122) {
                key = k - 32;
            } else {
                key = k;
            }
            mods = static_cast<unsigned int>(m);
        }

        key_combination(key_code k, unsigned int m) {
            if (k >= 97 && k <= 122) {
                key = k - 32;
            } else {
                key = k;
            }
            mods = m;
        }

        bool operator==(const key_combination& other) const {
            return key == other.key && mods == other.mods;
        }
    };
    
    struct key_combination_hasher {
        std::size_t operator()(const key_combination& V) const {
            return std::hash<key_code>{}(V.key) ^ std::hash<unsigned int>{}(V.mods);
        }
    };
    using key_map = std::unordered_map<key_combination, std::string, key_combination_hasher>;

    enum class mouse_button {
        left,
        right
    };

    namespace key {
        enum special {
            up = 265,
            down = 264,
            left = 263,
            right = 262,
            home = 268,
            end = 269,
            backspace = 259,
            page_up = 266,
            page_down = 267,
            enter = 257,
            tab = 258,
            equal = 61,
            minus = 45
        };
        
        key_combination seq(std::initializer_list<key_mods> mods, key_code key);        
        key_combination seq(key_code key);
    };
}