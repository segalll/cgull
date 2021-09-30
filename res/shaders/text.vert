#version 330 core

layout (location = 0) in vec2 a_vertex;
layout (location = 1) in vec2 a_position;
layout (location = 2) in uint a_glyph_id;

out float v_tex_coord_x;

struct glyph {
    vec2 advance;
    vec2 size;
    vec2 bearing;
    float tex_coord_x;
    float padding;
};

layout (std140) uniform glyphs {
    glyph glyph_array[1024]; // must be constant and current font (RobotoMono-Regular) has 876 glyphs
};

void main() {
    glyph g = glyph_array[a_glyph_id];

    vec2 pos = vec2(a_position.x + g.bearing.x, a_position.y - (g.size.y - g.bearing.y));
    // maybe * scale at end of each pos dimension
    // maybe make size g.size * scale

    v_tex_coord_x = g.tex_coord_x;
    gl_Position = vec4(a_vertex * g.size + pos, 0.0, 0.0);
}