#version 450
layout(location = 0) in vec3 inPosition;

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

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0f);
    gl_Position = (sceneData.projectionMatrix * sceneData.viewMatrix) * positionWorld;
}