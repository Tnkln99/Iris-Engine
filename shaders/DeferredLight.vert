#version 450

layout(location = 0) in vec2 inPosition; // Vertex position input
layout(location = 1) in vec2 inTexCoord; // Texture coordinate input

layout(location = 0) out vec2 texCoord; // Output texture coordinate to fragment shader

void main()
{
    texCoord = inTexCoord; // Pass texture coordinate to fragment shader
    gl_Position = vec4(inPosition, 0.0, 1.0); // Set position in clip space
}