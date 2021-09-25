#include "editor.h"

#include <variant>
#include <iostream>

/*
namespace {
    std::string to_utf8(char32_t codepoint) {
        auto& cvt = std::use_facet<std::codecvt<char32_t, char, std::mbstate_t>>(std::locale());

        std::mbstate_t state{};

        std::string out(cvt.max_length(), '\0');

        const char32_t* lastIn;
        char* lastOut;
        cvt.out(state, &codepoint, &codepoint + 1, lastIn, &out[0], &out[out.size()], lastOut);

        return out;
    }
}
*/

namespace cgull {
    editor::editor(key_map km) : keys(km) {}

    void editor::update(action a) {
        if (std::holds_alternative<char_action>(a)) {
            std::cout << std::get<char_action>(a).key << "\n";
        } else if (std::holds_alternative<special_key_action>(a)) {
            const auto ska = std::get<special_key_action>(a);
            if (ska.mods & static_cast<unsigned int>(key_mods::ctrl) && ska.mods & static_cast<unsigned int>(key_mods::alt)) {
                std::cout << "ctrl-alt+";
            }
            std::cout << ska.key << "\n";
        }
    }
}