#version 330 core

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

in vec3 position;

out vec3 ogPosition;

void main() {
    gl_Position = matProj * matView * matModel * vec4(position, 1);
    ogPosition = position;
}
