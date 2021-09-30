#version 330 core

in vec2 v_texcoord;
out vec4 f_color;

uniform sampler2D atlas;
uniform vec3 color;

void main() {
    f_color = vec4(color, texture(atlas, v_texcoord));
}