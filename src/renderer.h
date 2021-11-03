#pragma once

#include "coord.h"
#include "inputs.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

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
        std::unordered_map<key_code, glyph_info> glyph_map;
        unsigned int text_vao, text_vbo;
        unsigned int text_texture;
        unsigned int text_shader;
        unsigned int glyph_ubo;
        unsigned int font_atlas_width, font_atlas_height = 0;
        coord window_size;
        coord ubo_window_size;
        bool should_redraw;
        void render();
        void load_glyphs();
        std::vector<float> generate_batched_vertices(const std::string& text);
        void draw_text();
        void update_projection();
    };

    void render_loop(renderer* r, GLFWwindow* glfw_window);
}