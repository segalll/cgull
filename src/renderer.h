#pragma once

#include "editor.h"

#include <unordered_map>
#include <memory>

namespace {
    struct glyph_info {
        float ax, ay; // advance.x, advance.y
        float bw, bh; // bitmap.width, bitmap.rows
        float bl, bt; // bitmap_left, bitmap_top
        float tx, ty; // texture coordinates
    };
}

namespace cgull {
    struct renderer {
        renderer(coord window_size);
        void render(const editor& e);
        void draw_text(const buffer& buf);
        std::vector<glyph_info> glyph_list;
        std::unordered_map<key_code, unsigned int> glyph_map; // index into glyph_list
        std::unique_ptr<std::vector<std::vector<key_code>>> textToRender;
    private:
        void load_glyphs();
        void init_render_data();
        std::vector<float> generate_batched_vertices(const std::string& text);
        void update_projection();
        unsigned int text_vao, text_vbo;
        unsigned int text_texture;
        unsigned int text_shader;
        unsigned int glyph_ubo;
        unsigned int font_atlas_width, font_atlas_height = 0;
        coord gl_window_size;
    };
}