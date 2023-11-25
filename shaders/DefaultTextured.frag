//glsl version 4.5
#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 texCoord;

layout(set = 0, binding = 0) uniform SceneBuffer{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} sceneData;

layout(set = 1, binding = 0) uniform sampler2D ambient;
layout(set = 1, binding = 1) uniform sampler2D diffuse;
layout(set = 1, binding = 2) uniform sampler2D specular;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

//output write
layout (location = 0) out vec4 outColor;

void main()
{
    vec3 cameraPos = vec3(inverse(sceneData.viewMatrix)[3]);
    vec3 directionToLight = sceneData.lightPosition - fragPosWorld;
    float attenuation = 1.0f / dot(directionToLight, directionToLight); // distance squared

    vec3 finalLightColor = sceneData.lightColor.xyz * sceneData.lightColor.w * attenuation;
    vec3 ambientLight = sceneData.ambientLightColor.xyz * sceneData.ambientLightColor.w;
    vec3 diffuseLight = finalLightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)),0);

    // specular
    vec3 viewDir = normalize(cameraPos - fragPosWorld);
    vec3 reflectDir = reflect(-directionToLight, fragNormalWorld);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 10.0);


    vec3 ambientColor = texture(ambient,texCoord).xyz * ambientLight;
    vec3 diffuseColor = texture(diffuse,texCoord).xyz * diffuseLight;
    vec3 specularColor = texture(specular,texCoord).xyz * spec;

    vec4 finalColor = vec4((diffuseColor + specularColor + ambientColor) * fragColor, 1.0);
    outColor = finalColor;
}