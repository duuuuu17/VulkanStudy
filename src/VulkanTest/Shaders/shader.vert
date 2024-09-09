#version 450

// binging number is a resource location number that the descriptor set layout specified
// 'set' specify descripotr set number
layout(binding = 0) uniform UniformBufferObject{
 mat4 model;
 mat4 view;
 mat4 proj;
} ubo;
// layout: framebuffer, location = 0: frambuffer index
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}