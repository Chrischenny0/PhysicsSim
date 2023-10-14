#version 460

layout (location = 0) in vec2 pos;
layout (location = 2) in vec2 offset;

void main() {
    gl_Position = vec4(pos + offset, 0, 1);
}