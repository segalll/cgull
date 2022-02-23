#version 330 core

in vec2 v_texcoord;
in vec3 v_color;
out vec4 f_color;

uniform sampler2D atlas;

void main() {
    f_color = vec4(v_color, texture(atlas, v_texcoord).r);
}