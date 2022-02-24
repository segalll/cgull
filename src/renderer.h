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

struct text_vertex {
    float x, y;
    float tx, ty;
    float r, g, b;
};
} // namespace

namespace cgull {
struct cursor {
    unsigned int vao, vbo;
    float pos_x, pos_y;
    float height;
};

struct renderer {
    renderer(coord window_size, buffer* buf);

    buffer* text_buffer;
    unsigned int text_vao, text_vbo, text_texture, text_shader;
    unsigned int simple_shader;
    unsigned int proj_ubo;
    unsigned int font_atlas_width, font_atlas_height = 0;
    coord window_size;
    coord ubo_window_size;
    float face_height;
    unsigned int font_size;
    unsigned int desired_font_size;
    cursor text_cursor;
    std::vector<text_vertex> vertices;
    std::vector<uint32_t> row_indices; // row indices into vertex buffer
    std::vector<std::vector<float>> advances;
    std::vector<std::vector<float>> bearings;
    bool text_changed;
    float scroll_pos_y;
    float scroll_pos_x;
    std::mutex loop_mutex;
    std::condition_variable loop_cv;
    std::unordered_map<key_code, glyph_info> glyph_map;

    constexpr float max_scroll();
    constexpr float proper_cursor_pos(coord c);
    coord mouse_to_buffer(coord mouse_pos);
    void set_cursor_pos(coord c);
    void render();
    void load_glyphs();
    std::vector<text_vertex> generate_batched_vertices(const text& text_content);
    void update_font_size();
    void draw_text();
    void draw_cursor();
    void draw_selection();
    void update_projection();
};

void render_loop(renderer* r, GLFWwindow* glfw_window);
} // namespace cgull