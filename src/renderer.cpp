#include "renderer.h"

#include "shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <tuple>
#include <iostream>

namespace cgull {
    renderer::renderer(coord w_size) {
        window_size = w_size;
        ubo_window_size = w_size;

        if (gladLoadGL(glfwGetProcAddress) == 0) {
            throw std::runtime_error("failed to initialize OpenGL context\n");
        }

        load_glyphs();

        text_shader = create_shader("res/shaders/text.vert", "res/shaders/text.frag");

        glGenVertexArrays(1, &text_vao);
        glGenBuffers(1, &text_vbo);
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        const glm::mat4 m = glm::ortho(
            0.0f, static_cast<float>(ubo_window_size.col),
            0.0f, static_cast<float>(ubo_window_size.row)
        );
        glGenBuffers(1, &glyph_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, glyph_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(m), glm::value_ptr(m), GL_STATIC_DRAW);
        glUniformBlockBinding(text_shader, glGetUniformLocation(text_shader, "matrices"), 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, glyph_ubo);

        update_projection();

        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }

    void renderer::render() {
        glClear(GL_COLOR_BUFFER_BIT);
        if (ubo_window_size != window_size) {
            update_projection();
        }
        draw_text();
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

        FT_Set_Pixel_Sizes(face, 0, 16);

        unsigned int roww = 0;
        unsigned int rowh = 0;

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

        glGenTextures(1, &text_texture);
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            font_atlas_width,
            font_atlas_height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE, 
            nullptr
        );
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int x = 0;
        int y = 0;
        rowh = 0;

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

            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                x,
                y,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glyph_map.emplace(charcode, glyph_info{
                static_cast<float>(face->glyph->advance.x >> 6),
                static_cast<float>(face->glyph->advance.y >> 6),
                static_cast<float>(face->glyph->bitmap.width),
                static_cast<float>(face->glyph->bitmap.rows),
                static_cast<float>(face->glyph->bitmap_left),
                static_cast<float>(face->glyph->bitmap_top),
                static_cast<float>(x) / static_cast<float>(font_atlas_width),
                static_cast<float>(y) / static_cast<float>(font_atlas_height)
            });

            rowh = std::max(rowh, face->glyph->bitmap.rows);
            x += face->glyph->bitmap.width + 1;

            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void renderer::update_projection() {
        glViewport(0, 0, 1366, 642);
        ubo_window_size = window_size;
    }

    std::vector<float> renderer::generate_batched_vertices(const std::string& text) {
        std::vector<float> vertices;

        const float scale = 1.0f;

        float x = 50.0f;
        float y = 50.0f;

        for (char c : text) {
            const auto& glyph = glyph_map[static_cast<key_code>(c)];

            const float xpos = x + glyph.bl * scale;
            const float ypos = y - (glyph.bh - glyph.bt) * scale;

            const float w = glyph.bw * scale;
            const float h = glyph.bh * scale;

            const float tw = glyph.bw / static_cast<float>(font_atlas_width);
            const float th = glyph.bh / static_cast<float>(font_atlas_height);

            vertices.insert(vertices.end(), {
                xpos + w, ypos + h, glyph.tx + tw, glyph.ty,
                xpos,     ypos + h, glyph.tx,      glyph.ty,
                xpos,     ypos,     glyph.tx,      glyph.ty + th,
                xpos,     ypos,     glyph.tx,      glyph.ty + th,
                xpos + w, ypos,     glyph.tx + tw, glyph.ty + th,
                xpos + w, ypos + h, glyph.tx + tw, glyph.ty
            });
            x += glyph.ax * scale;
            y += glyph.ay * scale;
        }

        return vertices;
    }

    void renderer::draw_text() {
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glBindVertexArray(text_vao);
        glUseProgram(text_shader);
        std::string s = "hello world";
        const auto vertices = generate_batched_vertices(s);

        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        float c[] = { 1.0, 1.0, 1.0 };
        glUniform3fv(glGetUniformLocation(text_shader, "color"), 1, &c[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6 * s.size());
    }

    void render_loop(renderer* r, GLFWwindow* glfw_window) {
        glfwMakeContextCurrent(glfw_window);

        while (!glfwWindowShouldClose(glfw_window)) {
            if (r->should_redraw) {
                r->render();
                r->should_redraw = false;
                glfwSwapBuffers(glfw_window);
            }
        }
    }
}