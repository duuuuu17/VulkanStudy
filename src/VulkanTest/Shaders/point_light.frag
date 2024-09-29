#version 450

layout(location = 0) in vec2 fragOffset;

layout(location = 0) out vec4 outColor;

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

const float M_PI = 3.1415926538;
void main(){
	float dis = sqrt(dot(fragOffset,fragOffset));
	if(dis>=1.0)
		discard;
	float cosDis = 0.5 * (cos(dis * M_PI) + 1.0); // move up 1 then multiple half that range from 1-> 0
	outColor = vec4(push.lightColor.xyz + 0.5 * cosDis, cosDis); 
}
