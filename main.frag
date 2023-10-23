#version 330 core

out vec4 color;

in vec3 position;
in vec3 normal;
in vec2 texCoord0;

void main() {
    color = vec4(0.5 + 0.5 * position, 1);
}
