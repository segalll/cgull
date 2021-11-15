#include "editor.h"

#include <variant>
#include <iostream>

namespace {
    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
}

namespace cgull {
    editor::editor(coord w_size, key_map km) : window_size(w_size), keys(km) {}

    void editor::update(std::vector<action> actions) {
        for (const auto& a : actions) {
            if (std::holds_alternative<char_action>(a)) {
                buf.enter_char(std::get<char_action>(a).key);
            } else if (std::holds_alternative<special_key_action>(a)) {
                const auto ska = std::get<special_key_action>(a);
                if (ska.key == 257) { buf.new_line(); }
                else if (ska.mods & static_cast<unsigned int>(key_mods::shift) && ska.key == 258) { buf.unindent(); }
                else if (ska.key == 258) { buf.indent(); }
                else if (ska.key == 259) { buf.backspace(); }
                else if (ska.key == 263) { buf.cursor_left(); }
                else if (ska.key == 262) { buf.cursor_right(); }
                else if (ska.key == 265) { buf.cursor_up(); }
                else if (ska.key == 264) { buf.cursor_down(); }
                else if (ska.mods & static_cast<unsigned int>(key_mods::super) && ska.key == 83) { buf.save(); }
                else if (ska.mods & static_cast<unsigned int>(key_mods::super) && ska.key == 82) { compile(); }
            } else if (std::holds_alternative<resize_action>(a)) {
                const auto ra = std::get<resize_action>(a);
                window_size = ra.size;
            }
        }
    }

    void editor::compile() {
        std::cout << exec("java A.java") << "\n";
    }
}