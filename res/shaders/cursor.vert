#version 330 core

layout (location = 0) in vec2 a_vertex;

layout (std140) uniform matrices {
    mat4 proj;
};

uniform vec2 scroll;

void main() {
    gl_Position = proj * vec4(a_vertex.x + scroll.x, a_vertex.y - scroll.y, 0.0, 1.0);
}