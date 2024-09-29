#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 0) out vec4 outColor;

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
  mat4 modelMatrix; //  model transformation
  mat4 normalMatrix;
} push;

void main() {
	
	// ambient light
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);

	// get viewDirection: equal cameradiection
	vec3 viewDiection = normalize(ubo.inverseMatrix[3].xyz - fragPosWorld);
	vec3 specularLight = vec3(0.0);

	for(int i = 0; i < ubo.numLights; ++i)
	{	
		PointLights light = ubo.pointLights[i];
		// light position
		vec3 directionToLight = light.pointLightPosition.xyz - fragPosWorld;
		float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
		directionToLight = normalize(directionToLight);
		// theta
		float cosAngIncindence = max(dot(normalize(surfaceNormal), directionToLight),0);
		// point light intensity
		vec3 intensity = light.pointLightColor.xyz * light.pointLightColor.w * attenuation;
		// compute light color
		diffuseLight += intensity * cosAngIncindence;
	
		// specular light: use Blinn-Phong Model
		vec3 halfAngle = normalize(directionToLight + viewDiection);
		float blinnTerm = dot(halfAngle, surfaceNormal);
		blinnTerm = clamp(blinnTerm, 0 , 1);
		blinnTerm = pow(blinnTerm, 512.0);
		specularLight += intensity * blinnTerm;
	}	
	
	// diffuse = light * cos(theta)
	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}