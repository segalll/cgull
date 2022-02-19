#include "renderer.h"

#include "shader.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <numeric>
#include <stdexcept>
#include <tuple>

constexpr float x_offset = 20.0f;

namespace cgull {
renderer::renderer(coord w_size, buffer* buf) : text_buffer(buf) {
    window_size = w_size;
    ubo_window_size = w_size;

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        throw std::runtime_error("failed to initialize OpenGL context\n");
    }

    glGenTextures(1, &text_texture);

    font_size = 16; // initial font size
    desired_font_size = 16;
    load_glyphs();

    text_shader = create_shader("res/shaders/text.vert", "res/shaders/text.frag");
    simple_shader = create_shader("res/shaders/simple.vert", "res/shaders/simple.frag");

    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glGenVertexArrays(1, &text_cursor.vao);
    glGenBuffers(1, &text_cursor.vbo);
    glBindVertexArray(text_cursor.vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_cursor.vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    glGenBuffers(1, &proj_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, proj_ubo);
    glUniformBlockBinding(text_shader, glGetUniformLocation(text_shader, "matrices"), 0);
    glUniformBlockBinding(simple_shader, glGetUniformLocation(simple_shader, "matrices"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, proj_ubo);

    update_projection();

    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

constexpr float renderer::max_scroll() {
    const float visible_lines = (float)window_size.row / face_height;
    if (text_buffer->content.size() < visible_lines) {
        return 0.0f;
    }
    return (int)((float)text_buffer->content.size() - visible_lines) * face_height + 20.0f;
}

coord renderer::mouse_to_buffer(coord mouse_pos) {
    coord c;
    c.row = ((float)mouse_pos.row + scroll_pos_y - (face_height / 2)) / face_height;
    c.row = c.row > text_buffer->content.size() - 1 ? text_buffer->content.size() - 1 : c.row;
    c.col = 0;
    for (int i = 0; i < text_buffer->content[c.row].size(); i++) {
        if (mouse_pos.col >= vertices[row_indices[c.row] + (i * 24) + 4] + (advances[c.row][i] / 2.0f)) {
            c.col++;
        } else {
            break;
        }
    }

    return c;
}

void renderer::set_cursor_pos(coord c) {
    text_cursor.pos_x = c.col == 0
        ? x_offset
        : vertices[row_indices[c.row] + (c.col - 1) * 24 + 4] + advances[c.row][c.col - 1] - bearings[c.row][c.col - 1];
}

void renderer::render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (ubo_window_size != window_size) {
        update_projection();
    }
    if (desired_font_size != font_size) {
        update_font_size();
    }

    if (scroll_pos_y < 0.0f) {
        scroll_pos_y = 0.0f;
    } else if (scroll_pos_y > max_scroll()) {
        scroll_pos_y = max_scroll();
    }

    draw_text();
    draw_cursor();
    draw_selection();
}

void renderer::load_glyphs() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("failed to initialize FreeType");
    }

    FT_Face face;
    if (FT_New_Face(ft, "res/fonts/RobotoMono-Regular.ttf", 0, &face)) {
        throw std::runtime_error("failed to load font");
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    unsigned int roww = 0;
    unsigned int rowh = 0;
    font_atlas_width = 0;
    font_atlas_height = 0;

    unsigned int gindex;
    unsigned int charcode = FT_Get_First_Char(face, &gindex);
    while (gindex != 0) {
        if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
            std::cout << "failed to load glyph\n";
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
            continue;
        }

        if (roww + face->glyph->bitmap.width + 1 >= GL_MAX_TEXTURE_SIZE) {
            font_atlas_width = std::max(font_atlas_width, roww);
            font_atlas_height += rowh;
            roww = 0;
            rowh = 0;
        }
        roww += face->glyph->bitmap.width + 1;
        rowh = std::max(rowh, face->glyph->bitmap.rows);

        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }

    font_atlas_width = std::max(font_atlas_width, roww);
    font_atlas_height += rowh;

    glBindTexture(GL_TEXTURE_2D, text_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font_atlas_width, font_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int x = 0;
    int y = 0;
    rowh = 0;

    glyph_map.clear();

    charcode = FT_Get_First_Char(face, &gindex);
    while (gindex != 0) {
        if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
            continue;
        }

        if (x + face->glyph->bitmap.width + 1 >= GL_MAX_TEXTURE_SIZE) {
            y += rowh;
            rowh = 0;
            x = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED,
                        GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glyph_map.emplace(
            charcode,
            glyph_info{static_cast<float>(face->glyph->advance.x >> 6), static_cast<float>(face->glyph->advance.y >> 6),
                       static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows),
                       static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top),
                       static_cast<float>(x) / static_cast<float>(font_atlas_width),
                       static_cast<float>(y) / static_cast<float>(font_atlas_height)});

        rowh = std::max(rowh, face->glyph->bitmap.rows);
        x += face->glyph->bitmap.width + 2;

        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }

    face_height = face->size->metrics.height >> 6; // new line distance
    text_cursor.height = 2 * face_height - static_cast<float>(font_size);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void renderer::update_projection() {
    ubo_window_size = window_size;
    glViewport(0, 0, ubo_window_size.col, ubo_window_size.row);
    const glm::mat4 m =
        glm::ortho(0.0f, static_cast<float>(ubo_window_size.col), static_cast<float>(ubo_window_size.row), 0.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, proj_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(m), glm::value_ptr(m), GL_STATIC_DRAW);
}

std::vector<float> renderer::generate_batched_vertices(const text& text_content) {
    std::vector<float> v;

    float y = face_height;

    row_indices = {};
    advances = {};
    bearings = {};
    for (unsigned int r = 0; r < text_content.size(); r++) {
        row_indices.push_back(v.size());
        advances.push_back({});
        bearings.push_back({});
        float x = x_offset;
        for (unsigned int c = 0; c < text_content[r].size(); c++) {
            const auto& glyph = glyph_map[text_content[r][c]];

            const float xpos = x + glyph.bl;
            const float ypos = y - glyph.bt;

            const float w = glyph.bw;
            const float h = glyph.bh;

            if (r == text_buffer->cursor.row && c == text_buffer->cursor.col - 1) {
                text_cursor.pos_x = x + glyph.ax;
            }

            const float tw = glyph.bw / static_cast<float>(font_atlas_width);
            const float th = glyph.bh / static_cast<float>(font_atlas_height);

            v.insert(v.end(), {xpos + w,      ypos,          glyph.tx + tw, glyph.ty,      xpos,          ypos,
                               glyph.tx,      glyph.ty,      xpos,          ypos + h,      glyph.tx,      glyph.ty + th,
                               xpos,          ypos + h,      glyph.tx,      glyph.ty + th, xpos + w,      ypos + h,
                               glyph.tx + tw, glyph.ty + th, xpos + w,      ypos,          glyph.tx + tw, glyph.ty});
            advances[r].push_back(glyph.ax);
            bearings[r].push_back(glyph.bl);

            x += glyph.ax;
            y += glyph.ay;
        }
        y += face_height;
    }

    return v;
}

void renderer::update_font_size() {
    font_size = desired_font_size;
    load_glyphs();
}

void renderer::draw_text() {
    if (text_changed) {
        vertices = generate_batched_vertices(text_buffer->content);
        text_changed = false;
    }

    if (vertices.size() <= 0)
        return;

    glBindVertexArray(text_vao);
    glBindTexture(GL_TEXTURE_2D, text_texture);
    glUseProgram(text_shader);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    float c[] = {1.0f, 1.0f, 1.0f};
    glUniform3fv(glGetUniformLocation(text_shader, "color"), 1, c);
    float s[] = {scroll_pos_x, scroll_pos_y};
    glUniform2fv(glGetUniformLocation(text_shader, "scroll"), 1, s);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4); // 4 components per vertex
}

void renderer::draw_cursor() {
    float xpos = x_offset;
    float ypos = text_cursor.height + (face_height * text_buffer->cursor.row) - face_height - 1.0f; // just works
    if (text_buffer->cursor.col != 0) {
        xpos = text_cursor.pos_x;
    }

    const std::vector<float> vertices = {
        xpos + 1.0f, ypos,
        xpos - 1.0f, ypos,
        xpos - 1.0f, ypos + text_cursor.height,
        xpos - 1.0f, ypos + text_cursor.height,
        xpos + 1.0f, ypos + text_cursor.height,
        xpos + 1.0f, ypos,
    };
    glBindVertexArray(text_cursor.vao);
    glUseProgram(simple_shader);
    glBindBuffer(GL_ARRAY_BUFFER, text_cursor.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    float c[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glUniform4fv(glGetUniformLocation(simple_shader, "color"), 1, c);
    float s[] = {scroll_pos_x, scroll_pos_y};
    glUniform2fv(glGetUniformLocation(simple_shader, "scroll"), 1, s);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2); // 2 components per vertex
}

void renderer::draw_selection() {
    if (text_buffer->selection_start == std::nullopt || text_buffer->selection_start == text_buffer->cursor) {
        return;
    }

    std::vector<float> selection_vertices;

    coord front = *text_buffer->selection_start;
    coord back = text_buffer->cursor;
    if (front.row > back.row || (front.row == back.row && front.col > back.col)) {
        front = text_buffer->cursor;
        back = *text_buffer->selection_start;
    }
    for (int r = front.row; r <= back.row; r++) {
        float start_x, end_x;
        if (r == front.row && front.col != 0) {
            start_x = vertices[row_indices[r] + (front.col - 1) * 24 + 4] + advances[r][front.col - 1] - bearings[r][front.col - 1];
        } else {
            start_x = x_offset;
        }
        if (r + 1 <= back.row) {
            end_x = (text_buffer->content[r].size() == 0
                ? x_offset
                : vertices[row_indices[r] + (text_buffer->content[r].size() - 1) * 24]) + 12.0f;
        } else {
            end_x = back.col == 0 ? x_offset
                                  : vertices[row_indices[r] + (back.col - 1) * 24 + 4] + advances[r][back.col - 1];
        }
        float ypos = face_height * (r + 0.25);
        selection_vertices.insert(selection_vertices.end(), {
            end_x, ypos,
            start_x, ypos,
            start_x, ypos + face_height,
            start_x, ypos + face_height,
            end_x, ypos + face_height,
            end_x, ypos
        });
    }

    glBindVertexArray(text_cursor.vao);
    glUseProgram(simple_shader);
    glBindBuffer(GL_ARRAY_BUFFER, text_cursor.vbo);
    glBufferData(GL_ARRAY_BUFFER, selection_vertices.size() * sizeof(float), selection_vertices.data(),
                 GL_DYNAMIC_DRAW);

    float c[] = {1.0f, 1.0f, 1.0f, 0.3f};
    glUniform4fv(glGetUniformLocation(simple_shader, "color"), 1, c);
    float s[] = {scroll_pos_x, scroll_pos_y};
    glUniform2fv(glGetUniformLocation(simple_shader, "scroll"), 1, s);
    glDrawArrays(GL_TRIANGLES, 0, selection_vertices.size() / 2);
}

void render_loop(renderer* r, GLFWwindow* glfw_window) {
    std::unique_lock<std::mutex> lk(r->loop_mutex);

    glfwMakeContextCurrent(glfw_window);

    while (!glfwWindowShouldClose(glfw_window)) {
        r->render();
        glfwSwapBuffers(glfw_window);
        r->loop_cv.wait(lk);
    }
}
} // namespace cgull