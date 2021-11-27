#include "editor.h"

#include "nfd.hpp"

#include <variant>
#include <iostream>
#include <memory>

namespace {
    std::string exec(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
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
                key_combination kc {
                    std::get<special_key_action>(a).key,
                    std::get<special_key_action>(a).mods
                };
                if (keys.find(kc) == keys.end()) {
                    continue;
                }

                std::string command = keys[kc];
                if (command == "new-line") { buf.new_line(); }
                else if (command == "unindent") { buf.unindent(); }
                else if (command == "indent") { buf.indent(); }
                else if (command == "backspace") { buf.backspace(); }
                else if (command == "cursor-left") { buf.cursor_left(); }
                else if (command == "cursor-right") { buf.cursor_right(); }
                else if (command == "cursor-up") { buf.cursor_up(); }
                else if (command == "cursor-down") { buf.cursor_down(); }
                else if (command == "save") { buf.save(); }
                else if (command == "compile") { compile(); }
                else if (command == "zoom-in") { renderer_ptr->desired_font_size += 2; }
                else if (command == "zoom-out") { renderer_ptr->desired_font_size -= 2; }
                else if (command == "open") { open(); }
            } else if (std::holds_alternative<resize_action>(a)) {
                const auto ra = std::get<resize_action>(a);
                window_size = ra.size;
            }
        }
    }

    void editor::compile() {
        if (buf.file_path == std::nullopt) {
            buf.save();
        }
        std::string path = "";
        int start = 0;
        int end = buf.file_path.value().find("/");
        while (end != -1) {
            path += buf.file_path.value().substr(start, end - start) + "/";
            start = end + 1;
            end = buf.file_path.value().find("/", start);
        }
        std::string filename = buf.file_path.value().substr(start, end - start);

        std::string compileResult = exec("javac " + buf.file_path.value());
        std::string runResult = exec("java -cp " + path + " " + filename.substr(0, filename.length() - 5));
        std::cout << compileResult << "\n";
        std::cout << runResult << "\n";
    }

    void editor::open() {
        NFD::Guard nfdGuard;
        NFD::UniquePath outPath;
        nfdresult_t result = NFD::OpenDialog(outPath, nullptr, 0);
        if (result == NFD_OKAY) {
            buf = buffer(outPath.get());
        }
    }
}
