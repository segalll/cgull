#include "renderer.h"

#include <glad/gl.h>

#include <iostream>
#include <algorithm>

Renderer::Renderer() : textShader("res/shaders/text.vert", "res/shaders/text.frag") {
    loadGlyphs();
}

void Renderer::loadGlyphs() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "Failed to initialize FreeType\n";
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, "res/fonts/RobotoMono-Regular.ttf", 0, &face)) {
        std::cout << "Failed to load font\n";
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    unsigned int w, h = 0;
    unsigned int gindex;
    unsigned int charcode = FT_Get_First_Char(face, &gindex);
    while (gindex != 0) {
        if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
            std::cout << "Failed to load glyph\n";
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

        glyphMap.emplace(charcode, GlyphInfo{
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

void Renderer::renderText(const std::vector<std::vector<unsigned int>>& text) {
    const std::string s = "hello";
    drawQueue.push([&]() {
        textShader.use();
    });
}