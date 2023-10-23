#version 330 core

uniform float specularPow;

uniform vec3 dirLightDir;
uniform vec3 dirLightColor; // with intensity

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor; // with intensity
uniform vec2 spotLightAngleCos; // cos(phi), cos(theta)

out vec4 color;

in vec3 viewPosition;
in vec3 normal;
in vec2 texCoord0;

uniform sampler2D tex;

vec4 debugNormal(vec3 norm) {
    return vec4(0.5 + 0.5 * norm, 1);
}

void main() {
    vec3 viewDir = normalize(viewPosition);

    float dirDot = dot(-dirLightDir, normal);
    vec3 dirHalfway = normalize(viewDir + dirLightDir);
    // vec3 dirReflectDir = reflect(dirLightDir, normal);
    // float dirReflectDot = dot(-dirReflectDir, viewDir);
    float dirReflectDot = dot(-dirHalfway, normal);

    vec3 spotFall = viewPosition - spotLightPos;
    vec3 spotFallDir = normalize(spotFall);
    float spotDist2 = dot(spotFall, spotFall);
    float spotFallCos = dot(spotFallDir, spotLightDir);
    float spotCoverage = clamp(
        (spotFallCos - spotLightAngleCos.x)
        / (spotLightAngleCos.y - spotLightAngleCos.x),
        0, 1);

    float spotDot = dot(-spotFallDir, normal);
    vec3 spotHalfway = normalize(viewDir + spotFallDir);
    // vec3 spotReflectDir = reflect(spotFallDir, normal);
    // float spotReflectDot = dot(-spotReflectDir, viewDir);
    float spotReflectDot = dot(-spotHalfway, normal);

    float dirDiffuse = max(0, dirDot);
    float dirSpecular = pow(max(0, dirReflectDot), specularPow);

    float spotDiffuse = max(0, spotDot);
    float spotSpecular = pow(max(0, spotReflectDot), specularPow);

    vec3 baseColor = texture(tex, texCoord0).xyz;
    vec3 dirColor = (dirDiffuse + dirSpecular) * baseColor * dirLightColor;
    vec3 spotColor = (spotDiffuse + spotSpecular) * baseColor * spotLightColor;
    color = vec4(dirColor + spotColor, 1);
}
