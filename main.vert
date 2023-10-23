#version 330 core

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

// To not calculate it in the shader
uniform mat4 matNormal;

uniform float morphProgress;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord0;

out vec3 viewPosition;
out vec3 normal;
out vec2 texCoord0;

void main() {
    vec3 nextPos = normalize(inPosition);
    vec3 nextNormal = nextPos; // morphing to a sphere

    vec3 pos = mix(inPosition, nextPos, morphProgress);
    vec4 vp = matView * matModel * vec4(pos, 1);
    viewPosition = vp.xyz;
    gl_Position = matProj * vp;

    vec3 immNormal = mix(inNormal, nextNormal, morphProgress);
    normal = (matNormal * vec4(immNormal, 0)).xyz;

    texCoord0 = inTexCoord0;
}
