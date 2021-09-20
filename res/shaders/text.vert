#version 330 core

layout (location = 0) in vec2 a_vertex;
layout (location = 1) in vec2 a_position;
layout (location = 2) in uint a_glyph_id;

out float v_tex_coord_x;

struct glyph {
    vec2 size;
    vec2 anchor;
    float tex_coord_x;
    float padding;
};

layout (std140) uniform glyphs {
    glyph glyph_array[];
};

void main() {
    v_tex_coord_x = glyph_array[a_glyph_id].tex_coord_x;
    gl_Position = vec4(a_vertex * glyph_array[a_glyph_id].size + a_position, 0.0, 0.0);
}