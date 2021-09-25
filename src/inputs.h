#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <initializer_list>
#include <bitset>

namespace cgull {
    using key_code = unsigned int;
    using key_seq = std::vector<key_code>;
    
    struct key_seq_hasher {
        key_code operator()(const std::vector<key_code>& V) const {
            int hash = V.size();

            for (const auto& i : V) {
                hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };
    using key_map = std::unordered_map<key_seq, std::string, key_seq_hasher>;

    key_map make_key_map(std::initializer_list<std::pair<key_seq, std::string>> args);

    enum class mouse_button {
        left,
        right
    };

    enum class key_mods {
        none = 0x00,
        shift = 0x01,
        ctrl = 0x02,
        alt = 0x04,
        super = 0x08,
        caps_lock = 0x10,
        num_lock = 0x20
    };
}