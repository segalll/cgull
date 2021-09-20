#version 330 core

in float v_tex_coord_x;
out vec4 f_color;

uniform sampler2D atlas;
uniform vec3 color;

void main() {
    f_color = vec4(color, texture(atlas, vec2(v_tex_coord_x, 0.0)));
}