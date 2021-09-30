#include "renderer.h"

#include "shader.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h> // sadly I must include for glfwGetProcAddress
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>
#include <iostream>

namespace cgull {
    renderer::renderer() {
        if (gladLoadGL(glfwGetProcAddress) == 0) {
            throw std::runtime_error("failed to initialize OpenGL context\n");
        }
        load_glyphs();
        init_render_data();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        generate_batched_vertices("hello");
    }

    void renderer::render(const editor& app) {
        glClear(GL_COLOR_BUFFER_BIT);
        draw_text(app.buf, app.window_size);
    }

    void renderer::draw_text(const buffer &buf, coord size) {
        glBindVertexArray(text_vao);
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glUseProgram(text_shader);
        const auto vertices = generate_batched_vertices("hey");
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        float c[] = { 1.0, 1.0, 1.0 };
        glUniform3fv(glGetUniformLocation(text_shader, "color"), 1, &c[0]);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 6, 1);
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

        FT_Set_Pixel_Sizes(face, 0, 48);

        unsigned int gindex;
        unsigned int charcode = FT_Get_First_Char(face, &gindex);
        while (gindex != 0) {
            if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
                std::cout << "failed to load glyph\n";
            } else {
                font_atlas_width += face->glyph->bitmap.width;
                font_atlas_height = std::max(font_atlas_height, face->glyph->bitmap.rows);
            }

            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        glGenTextures(1, &text_texture);
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            font_atlas_width,
            font_atlas_height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE, 
            0
        );

        unsigned int x = 0;
        charcode = FT_Get_First_Char(face, &gindex);
        while (gindex != 0) {
            if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
                continue;
            }

            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                x,
                0,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glyph_list.push_back(glyph_info{
                static_cast<float>(face->glyph->advance.x >> 6),
                static_cast<float>(face->glyph->advance.y >> 6),
                static_cast<float>(face->glyph->bitmap.width),
                static_cast<float>(face->glyph->bitmap.rows),
                static_cast<float>(face->glyph->bitmap_left),
                static_cast<float>(face->glyph->bitmap_top),
                static_cast<float>(x) / static_cast<float>(font_atlas_width)
            });

            glyph_map[charcode] = glyph_list.size() - 1;

            x += face->glyph->bitmap.width;

            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void renderer::init_render_data() {
        text_shader = create_shader("res/shaders/text.vert", "res/shaders/text.frag");

        const std::vector<float> vertices{
            // pos    // tex
            1.0, 1.0, 1.0, 1.0,
            0.0, 1.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0,
            1.0, 0.0, 1.0, 0.0,
            1.0, 1.0, 1.0, 1.0
        };

        glGenVertexArrays(1, &text_vao);
        glGenBuffers(1, &text_vbo);
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glGenBuffers(1, &glyph_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, glyph_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glyph_list), glyph_list.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    std::vector<float> renderer::generate_batched_vertices(const std::string& text) {
        std::vector<float> vertices;

        const float scale = 1.0f;

        float x = 0.2f;
        float y = 0.2f;

        for (char c : text) {
            const auto& glyph = glyph_list[glyph_map[static_cast<key_code>(c)]];

            const float xpos = x + glyph.bl * scale;
            const float ypos = y - (glyph.bh - glyph.bt) * scale;

            const float w = glyph.bw * scale;
            const float h = glyph.bh * scale;

            const float tw = glyph.bw / static_cast<float>(font_atlas_width);
            const float th = glyph.bh / static_cast<float>(font_atlas_height);

            vertices.insert(vertices.end(), {
                xpos + w, ypos + h, glyph.tx + tw, 0.0f,
                xpos,     ypos + h, glyph.tx,      0.0f,
                xpos,     ypos,     glyph.tx,      th,
                xpos,     ypos,     glyph.tx,      th,
                xpos + w, ypos,     glyph.tx + tw, th,
                xpos + w, ypos + h, glyph.tx + tw, 0.0f
            });
        }

        return vertices;
    }
}