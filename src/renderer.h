#pragma once

#include "shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <functional>
#include <memory>

namespace {
    struct GlyphInfo {
        float ax, ay; // advance.x, advance.y
        float bw, bh; // bitmap.width, bitmap.rows
        float bl, bt; // bitmap_left, bitmap_top
        float tx; // texture coordinate x offset
    };
}

class Renderer {
public:
    Renderer();

    void renderText(const std::vector<std::vector<unsigned int>>& text);
private:
    std::unordered_map<unsigned int, GlyphInfo> glyphMap;
    std::queue<std::function<void()>> drawQueue;
    Shader textShader;
    std::unique_ptr<std::vector<std::vector<unsigned int>>> textToRender;

    void loadGlyphs();
};