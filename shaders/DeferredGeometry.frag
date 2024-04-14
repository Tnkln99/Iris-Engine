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

layout(set = 1, binding = 0) uniform sampler2D ambient;
layout(set = 1, binding = 1) uniform sampler2D diffuse;
layout(set = 1, binding = 2) uniform sampler2D specular;

//output write
layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outSpeculer;
layout (location = 3) out vec4 outPosition;


void main() {
    vec3 cameraPos = vec3(inverse(sceneData.viewMatrix)[3]);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 ambientLight = sceneData.ambientLightColor.xyz * sceneData.ambientLightColor.w;
    vec3 diffuseLight = vec3(0,0,0);
    vec3 specLight = vec3(0,0,0);
    for(int i = 0; i < sceneData.numLights; ++i){
        vec4 lightPos = sceneData.pointLights[i].position;
        vec4 lightColor = sceneData.pointLights[i].color;

        vec3 directionToLight = lightPos.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(directionToLight, directionToLight);// distance squared
        float cosAngIncidence = dot(surfaceNormal, directionToLight);
        cosAngIncidence = clamp(cosAngIncidence, 0, 1);

        // diffuse
        vec3 finalLightColor = lightColor.xyz * lightColor.w * attenuation;
        diffuseLight += finalLightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

        // specular
        vec3 viewDir = normalize(cameraPos - fragPosWorld);
        vec3 halfAngle = normalize(directionToLight + viewDir);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = cosAngIncidence != 0.0 ? blinnTerm : 0.0;
        blinnTerm = pow(blinnTerm, 32.0);
        specLight += lightColor.xyz * attenuation * blinnTerm;
    }


    vec3 ambientColor = texture(ambient,texCoord).xyz * ambientLight;
    vec3 diffuseColor = texture(diffuse,texCoord).xyz * diffuseLight;
    vec3 specularColor = texture(specular,texCoord).xyz * specLight;

    vec4 finalAlbedo = vec4((diffuseColor + ambientColor) * fragColor, 1.0);
    outAlbedo = finalAlbedo;
    vec4 finalSpecular = vec4(specularColor * fragColor, 1.0);
    outSpeculer = finalSpecular;

    outPosition = vec4(fragNormalWorld, 1);
    outNormal = vec4(fragNormalWorld, 1);
}
