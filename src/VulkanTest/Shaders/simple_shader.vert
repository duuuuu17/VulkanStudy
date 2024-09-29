#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

struct PointLights{
	vec4 pointLightPosition;
	vec4 pointLightColor;
};

layout(set = 0, binding = 0) uniform GLoableUbo {
  mat4 projectionMatrix; // projection * view
  mat4 viewMatrix; // projection * view
  mat4 inverseMatrix; 
  vec4 ambientLightColor; // w is intensity
  PointLights pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // model transformation
  mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorldSpace =  push.modelMatrix * vec4(position, 1.0f);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * positionWorldSpace;
	
	fragPosWorld = positionWorldSpace.xyz;
	fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragColor = color;

}