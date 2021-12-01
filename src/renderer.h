#pragma once

#include "buffer.h"
#include "coord.h"
#include "inputs.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <condition_variable>
#include <unordered_map>

namespace {
struct glyph_info {
    float ax, ay; // advance.x, advance.y
    float bw, bh; // bitmap.width, bitmap.rows
    float bl, bt; // bitmap_left, bitmap_top
    float tx, ty; // texture coordinates
};
} // namespace

namespace cgull {
struct cursor {
    unsigned int vao, vbo, shader;
    float pos_x, pos_y;
    float height;
};

struct renderer {
    renderer(coord window_size, buffer* buf);

    buffer* text_buffer;
    unsigned int text_vao, text_vbo, text_texture, text_shader;
    unsigned int proj_ubo;
    unsigned int font_atlas_width, font_atlas_height = 0;
    coord window_size;
    coord ubo_window_size;
    float face_height;
    unsigned int font_size;
    unsigned int desired_font_size;
    cursor text_cursor;
    std::mutex loop_mutex;
    std::condition_variable loop_cv;
    std::unordered_map<key_code, glyph_info> glyph_map;

    void render();
    void load_glyphs();
    std::vector<float> generate_batched_vertices(const text& text_content);
    void update_font_size();
    void draw_text();
    void draw_cursor();
    void update_projection();
};

void render_loop(renderer* r, GLFWwindow* glfw_window);
} // namespace cgull