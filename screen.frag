#version 330 core

in vec3 viewPosition;
in vec3 normal;
in vec2 texCoord0;

uniform sampler2D gBaseColor;
uniform sampler2D gNormal;

uniform sampler2D gDepth;
uniform vec4 viewport; // width, height, zNear, zFar

uniform float specularPow;

uniform vec3 dirLightDir;
uniform vec3 dirLightColor; // with intensity

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor; // with intensity
uniform vec2 spotLightAngleCos; // cos(phi), cos(theta)

out vec4 fragColor;

vec4 debugNormal(vec3 norm) {
    return vec4(0.5 + 0.5 * norm, 1);
}

float restoreZ(float depth) {
    // (a * z + b) / z = depth
    // a + b / z = depth
    // a + b / zNear = 1
    // a + b / zFar = 0
    // b = -zFar * a
    // a - a * zFar / zNear = 1
    // a * (zNear - zFar) / zNear = 1
    // a = zNear / (zNear - zFar)
    // b = -zFar * zNear / (zNear - zFar)
    // a + b / z = depth
    // b / z = depth - a
    // z = b / (depth - a)
    float a = viewport.z / (viewport.z - viewport.w);
    float b = -viewport.w * a;
    return b / (depth - a);
}

void main() {
    vec4 baseColor = texture(gBaseColor, texCoord0);
    vec3 normal = texture(gNormal, texCoord0).xyz;
    float depth = texture(gDepth, texCoord0).x;
    if (baseColor.w < 1) discard;

    float random = fract(314 * sin(dot(gl_FragCoord.xy, vec2(3.14, 2.0))));
    fragColor = vec4(random, 0, 0, 1);
    return;

    vec3 viewPosition;
    viewPosition.z = restoreZ(depth);
    viewPosition.xy = viewPosition.z * (texCoord0.xy * 2 - 1);

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

    vec3 dirColor = (dirDiffuse + dirSpecular) * baseColor.xyz * dirLightColor;
    vec3 spotColor = (spotDiffuse + spotSpecular) * baseColor.xyz * spotLightColor;

    // fragColor = vec4(dirColor + spotColor, baseColor.w);
    fragColor = debugNormal(spotFallDir);
}
