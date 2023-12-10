//we will be using glsl version 4.5 syntax
#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout (location = 3) out vec2 texCoord;

struct PointLight {
    vec4 position; // ignore w
    vec4 color;  // w is intensity
};

layout(set = 0, binding = 0) uniform SceneBuffer{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 ambientLightColor;
    int numLights;
    PointLight pointLights[10];
} sceneData;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;


void main()
{
    vec4 positionWorld = push.modelMatrix * vec4(pos, 1.0f);
    gl_Position = (sceneData.projectionMatrix * sceneData.viewMatrix) * positionWorld;
    fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    texCoord = uv;
}