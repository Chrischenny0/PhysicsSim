#version 460

layout (location = 0) in vec2 pos;
layout (location = 2) in vec2 offset;
out vec2 coords;
uniform vec2 size;
void main() {
    gl_Position = vec4(pos + offset, 0, 1);
    coords = (offset + 1) * size * 0.5;
}