#version 330 core

in vec2 texCoord0;

uniform sampler2D gBaseColor;
uniform sampler2D gNormal;

uniform sampler2D gDepth;
uniform vec4 viewport; // width, height, zNear, zFar

uniform sampler2D noiseTexture;

uniform vec4 ambient; // with intensity + occlusion radius
uniform int ssaoSamples;

uniform float specularPow;

uniform vec3 dirLightDir;
uniform vec3 dirLightColor; // with intensity

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor; // with intensity
uniform vec2 spotLightAngleCos; // cos(phi), cos(theta)

out vec4 fragColor;

vec3 viewPos;
vec3 viewDir;
vec3 normal;
float depth;

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
    return -b / (depth - a);
}

float calcAmbient() {
    int misses = 1;
    vec2 randomSamplePos = texCoord0;
    for (int i = 0; i < ssaoSamples; ++i) {
        vec4 randomVec = texture(noiseTexture, randomSamplePos);
        randomSamplePos = randomVec.zw;

        randomVec = 2 * randomVec - 1;
        if (dot(randomVec, randomVec) > 1) continue;

        vec3 offset = randomVec.xyz;
        float dotNorm = dot(offset, normal);
        if (dotNorm < 0)
            offset -= 2 * dotNorm * normal;

        vec3 pos = viewPos + ambient.w * offset;
        vec2 uv = pos.xy / pos.z * 0.5 + 0.5;
        float sampledDepth = texture(gDepth, uv).x;
        float sampledZ = restoreZ(sampledDepth);
        if (pos.z >= sampledZ)
            misses++;
    }
    return misses / float(1 + ssaoSamples);
}

// diffuse + specular
float calcDir() {
    float dirDot = dot(-dirLightDir, normal);
    vec3 halfway = normalize(viewDir + dirLightDir);
    // vec3 dirReflectDir = reflect(dirLightDir, normal);
    // float dirReflectDot = dot(-dirReflectDir, viewDir);
    float reflectDot = dot(-halfway, normal);

    float diffuse = max(0, dirDot);
    float specular = pow(max(0, reflectDot), specularPow);

    return diffuse + specular;
}

float calcSpot() {
    vec3 fall = viewPos - spotLightPos;
    vec3 fallDir = normalize(fall);
    float dist2 = dot(fall, fall);
    float fallCos = dot(fallDir, spotLightDir);
    float spotCoverage = clamp(
        (fallCos - spotLightAngleCos.x)
        / (spotLightAngleCos.y - spotLightAngleCos.x),
        0, 1);

    float spotDot = dot(-fallDir, normal);
    vec3 halfway = normalize(viewDir + fallDir);
    // vec3 reflectDir = reflect(fallDir, normal);
    // float reflectDot = dot(-reflectDir, viewDir);
    float reflectDot = dot(-halfway, normal);

    float diffuse = max(0, spotDot);
    float specular = pow(max(0, reflectDot), specularPow);

    return (diffuse + specular) / dist2;
}

void main() {
    vec4 baseColor = texture(gBaseColor, texCoord0);
    depth = texture(gDepth, texCoord0).x;
    normal = 2 * texture(gNormal, texCoord0).xyz - 1;
    if (baseColor.w < 1) discard;

    viewPos.z = restoreZ(depth);
    viewPos.xy = viewPos.z * (texCoord0.xy * 2 - 1);

    viewDir = normalize(viewPos);

    vec3 ambientColor = calcAmbient() * ambient.xyz;
    vec3 dirColor = calcDir() * dirLightColor;
    vec3 spotColor = calcSpot() * spotLightColor;
    vec3 combined = ambientColor + dirColor + spotColor;

    fragColor = vec4(combined * baseColor.xyz, baseColor.w);
}
