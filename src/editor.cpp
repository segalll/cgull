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
    editor::editor(coord w_size, key_map km) : window_size(w_size), keys(km) {}

    void editor::update(std::vector<action> actions) {
        for (const auto& a : actions) {
            if (std::holds_alternative<char_action>(a)) {
                buf.enter_char(std::get<char_action>(a).key);
            } else if (std::holds_alternative<special_key_action>(a)) {
                const auto ska = std::get<special_key_action>(a);
                if (ska.key == 257) {
                    buf.new_line();
                } else if (ska.key == 259) {
                    buf.backspace();
                }
            } else if (std::holds_alternative<resize_action>(a)) {
                const auto ra = std::get<resize_action>(a);
                window_size = ra.size;
            }
        }
    }
}