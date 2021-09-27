#include "renderer.h"

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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }

    void renderer::render(const editor& app) {
        glClear(GL_COLOR_BUFFER_BIT);
        draw_text(app.buf, app.window_size);
    }

    void renderer::draw_text(const buffer &buf, coord size) {}

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

        unsigned int w, h = 0;
        unsigned int gindex;
        unsigned int charcode = FT_Get_First_Char(face, &gindex);
        while (gindex != 0) {
            if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
                std::cout << "failed to load glyph\n";
            } else {
                w += face->glyph->bitmap.width;
                h = std::max(h, face->glyph->bitmap.rows);
            }

            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

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

            glyph_map.emplace(charcode, glyph_info{
                static_cast<float>(face->glyph->advance.x >> 6),
                static_cast<float>(face->glyph->advance.y >> 6),
                static_cast<float>(face->glyph->bitmap.width),
                static_cast<float>(face->glyph->bitmap.rows),
                static_cast<float>(face->glyph->bitmap_left),
                static_cast<float>(face->glyph->bitmap_top),
                static_cast<float>(x) / static_cast<float>(w)
            });

            x += face->glyph->bitmap.width;

            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }
}