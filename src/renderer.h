#pragma once

#include "editor.h"

#include <unordered_map>
#include <queue>
#include <functional>
#include <memory>

namespace {
    struct glyph_info {
        float ax, ay; // advance.x, advance.y
        float bw, bh; // bitmap.width, bitmap.rows
        float bl, bt; // bitmap_left, bitmap_top
        float tx; // texture coordinate x offset
    };
}

namespace cgull {
    struct renderer {
        renderer();
        void render(const editor& app);
        void draw_text(const buffer& buf, coord size);
        std::unordered_map<key_code, glyph_info> glyph_map;
        std::queue<std::function<void()>> draw_queue;
        std::unique_ptr<std::vector<std::vector<key_code>>> textToRender;
    private:
        void load_glyphs();
    };
}