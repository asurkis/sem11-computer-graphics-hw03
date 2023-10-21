#version 330 core

out vec4 color;

void main() {
    color = vec4(gl_FragCoord.xy / 1000, 0, 1);
}
