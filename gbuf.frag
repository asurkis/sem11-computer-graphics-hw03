#version 330 core

uniform bool isTextured;
uniform vec4 colorFactor;

layout (location = 0) out vec4 gBaseColor;
layout (location = 1) out vec4 gNormal;

in vec3 normal;
in vec2 texCoord0;

uniform sampler2D tex;

void main() {
    vec3 baseColor = isTextured ? texture(tex, texCoord0).xyz : vec3(1);
    baseColor *= colorFactor.xyz;
    gBaseColor = vec4(baseColor, 1);
    gNormal = vec4(0.5 * normal + 0.5, 0);
}
