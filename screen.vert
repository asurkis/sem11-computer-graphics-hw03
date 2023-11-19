#version 330 core

layout (location = 0) in vec2 inTexCoord0;

out vec2 texCoord0;

void main() {
    texCoord0 = inTexCoord0;
    gl_Position = vec4(2 * inTexCoord0 - 1, 0, 1);
}

