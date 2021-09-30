#version 330 core

layout (location = 0) in vec2 a_vertex;
layout (location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

void main() {
    v_texcoord = a_texcoord;
    gl_Position = vec4(a_vertex, 0.0, 0.0);
}