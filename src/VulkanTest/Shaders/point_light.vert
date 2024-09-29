#version 450
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;

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

layout(push_constant) uniform Push{
	vec4 lightPosition;
	vec4 lightColor;
	float radius;
}push;

void main(){
	fragOffset = OFFSETS[gl_VertexIndex];
	
	// OpenGL Tutorial introduce:
	//vec3 cameraRightWorld = {ubo.viewMatrix[0][0],ubo.viewMatrix[1][0],ubo.viewMatrix[2][0]};
	//vec3 cameraUpWorld = {ubo.viewMatrix[0][1],ubo.viewMatrix[1][1],ubo.viewMatrix[2][1]};
	// compute the Billboard relative to Camera position
	//vec3 positionWorld = push.lightPosition.xyz 
	//	+ push.radius * cameraRightWorld * fragOffset.x 
	//	+ push.radius * cameraUpWorld * fragOffset.y ;
	//gl_Position = ubo.projectionMatrix  * vec4(positionWorld,1.0);

	// another method is to first transform light position to camera space
	 vec4 lightCameraSpace = ubo.viewMatrix * vec4(push.lightPosition.xyz, 1.0);
	 lightCameraSpace = lightCameraSpace + push.radius * vec4(fragOffset, 0.0,0.0);
	 gl_Position = ubo.projectionMatrix  * lightCameraSpace;
}
