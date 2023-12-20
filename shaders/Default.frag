//glsl version 4.5
#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 texCoord;

struct PointLight {
    vec4 position; // ignore w
    vec4 color;  // w is intensity
};

layout(set = 0, binding = 0) uniform SceneBuffer{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 ambientLightColor;
    int numLights;
    PointLight pointLights[50];
} sceneData;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

//output write
layout (location = 0) out vec4 outColor;

void main()
{
    vec3 cameraPos = vec3(inverse(sceneData.viewMatrix)[3]);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 ambientLight = sceneData.ambientLightColor.xyz * sceneData.ambientLightColor.w;
    vec3 diffuse = vec3(0,0,0);
    vec3 speculer = vec3(0,0,0);

    for(int i = 0; i < sceneData.numLights; ++i){
        vec4 lightPos = sceneData.pointLights[i].position;
        vec4 lightColor = sceneData.pointLights[i].color;
        vec3 directionToLight = lightPos.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(directionToLight, directionToLight); // distance squared
        float cosAngIncidence = dot(surfaceNormal, directionToLight);
        cosAngIncidence = clamp(cosAngIncidence, 0, 1);

        // diffuse
        vec3 finalLightColor = lightColor.xyz * lightColor.w * attenuation;
        diffuse += finalLightColor * max(dot(surfaceNormal, normalize(directionToLight)),0);
        // specular
        vec3 viewDir = normalize(cameraPos - fragPosWorld);
        vec3 halfAngle = normalize(directionToLight + viewDir);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = cosAngIncidence != 0.0 ? blinnTerm : 0.0;
        blinnTerm = pow(blinnTerm, 32.0);
        speculer += lightColor.xyz * attenuation * blinnTerm;
    }

    // I dont add the specular light to calculations because it looks bad
    outColor = vec4((diffuse + speculer + ambientLight) * fragColor, 1.0);
}