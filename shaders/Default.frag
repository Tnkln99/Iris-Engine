//glsl version 4.5
#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 texCoord;

layout(set = 0, binding = 0) uniform CameraBuffer{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} cameraData;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

//output write
layout (location = 0) out vec4 outColor;

void main()
{
    vec4 ambientLightColor = vec4(1.f, 1.f, 1.f, .02f); // w is intesity
    vec3 lightPosition = vec3(0,1,1);
    vec4 lightColor = vec4(1,1,1,1);

    vec3 directionToLight = lightPosition - fragPosWorld;
    float attenuation = 1.0f / dot(directionToLight, directionToLight); // distance squared

    vec3 finalLightColor = lightColor.xyz * lightColor.w * attenuation;
    vec3 ambientLight = ambientLightColor.xyz * ambientLightColor.w;
    vec3 diffuseLight = finalLightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)),0);

    outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}