#version 450

layout(location = 0) in vec2 texCoord; // Interpolated texture coordinate from vertex shader
layout(location = 0) out vec4 outColor; // Output color of the fragment

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput colorMap;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput normalMap;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput specularMap;
layout(input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput positionMap;

void main()
{
    outColor = vec4(subpassLoad(colorMap).rgb, 1.0);
}