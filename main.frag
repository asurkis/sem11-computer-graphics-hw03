#version 330 core

// width, height, tan(fov), gamma
uniform vec4 viewport;

uniform vec3 dirLightDir;
uniform vec3 dirLightColor; // with intensity

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor; // with intensity
uniform vec2 spotLightAngleCos;

out vec4 color;

in vec3 position;
in vec3 normal;
in vec2 texCoord0;

uniform sampler2D tex;

void main() {
    vec2 xy = (2.0 * gl_FragCoord.xy / viewport.xy - 1.0) * vec2(viewport.x / viewport.y, 1) * viewport.z;
    vec3 viewDir = normalize(vec3(xy, -1));
    vec3 halfway = 0.5 * (viewDir + dirLightDir);

    vec3 dirReflectDir = reflect(-dirLightDir, normal);
    float dirDot = dot(-dirLightDir, normal);
    float dirReflectDot = dot(dirReflectDir, viewDir);

    float dirDiffuse = max(0, dirDot);
    float dirSpecular = pow(max(0, dirReflectDot), 4);

    vec3 albedo = texture(tex, texCoord0).xyz;
    vec3 dirColor = (dirDiffuse + dirSpecular) * albedo * dirLightColor;
    // color = vec4(viewport.w * dirColor, 1);
    color = vec4(dirReflectDot, 0, 0, 1);
}
