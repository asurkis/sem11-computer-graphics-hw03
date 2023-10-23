#version 330 core

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

// To not calculate it in the shader
uniform mat4 matNormal;

uniform float morphProgress;

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord0;

out vec3 position;
out vec3 normal;
out vec2 texCoord0;

void main() {
    vec3 next = normalize(inPosition);
    vec3 pos = mix(inPosition, next, morphProgress);
    gl_Position = matProj * matView * matModel * vec4(pos, 1);
    position = inPosition;
    // normal = (matNormal * vec4(inNormal, 0)).xyz;
    texCoord0 = inTexCoord0;
}
