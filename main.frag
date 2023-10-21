#version 330 core

out vec4 color;

in vec3 ogPosition;

void main() {
    color = vec4(0.5 + 0.5 * ogPosition, 1);
}
