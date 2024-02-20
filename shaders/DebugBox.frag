#version 450
layout(location = 0) out vec4 outColor;

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
    outColor = vec4(0.0, 0.0, 1.0, 1.0); // blue color
}