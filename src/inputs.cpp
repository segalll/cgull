#include "inputs.h"

#include <stdexcept>

namespace cgull {
    key_map make_key_map(std::initializer_list<std::pair<key_seq, std::string>> args) {
        key_map map = {};
        for (auto item : args) {
            auto kseq = key_seq{};
            for (auto kcode : item.first) {
                if (!map[kseq].empty()) {
                    throw std::runtime_error("ambiguous binding");
                }
                kseq.push_back(kcode);
            }
            auto res = map.emplace(std::move(kseq), std::move(item.second));
            if (!res.second) {
                throw std::runtime_error("duplicate binding");
            }
        }
        return map;
    }
}