#version 330 core

layout (location = 0) in vec2 a_vertex;
layout (location = 1) in vec2 a_texcoord;

layout (std140) uniform matrices {
    mat4 proj;
};

out vec2 v_texcoord;

uniform vec2 scroll;

void main() {
    v_texcoord = a_texcoord;
    gl_Position = proj * vec4(a_vertex.x + scroll.x, a_vertex.y - scroll.y, 0.0, 1.0);
}