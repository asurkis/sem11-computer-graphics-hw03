#version 330 core

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

uniform float morphProgress;

in vec3 position;
in vec3 normal;
in vec2 texCoord0;

out vec3 ogPosition;

void main() {
    vec3 next = normalize(position);
    vec3 pos = mix(position, next, morphProgress);
    gl_Position = matProj * matView * matModel * vec4(pos, 1);
    ogPosition = position;
}
