#version 330 core

in vec3 viewPosition;
in vec3 normal;
in vec2 texCoord0;

uniform sampler2D fragBaseColor;
uniform sampler2D fragNormal;

out vec4 color;

vec4 debugNormal(vec3 norm) {
    return vec4(0.5 + 0.5 * norm, 1);
}

void main() {
    color = texture(fragBaseColor, texCoord0);
    color.w = 1;
}
