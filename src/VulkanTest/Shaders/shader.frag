#version 450
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) out vec4 outColor;

void main()
{
	// outColor = vec4(fragColor, 1.0);
	// outColor = vec4(fragTexCoord, 0.0, 1.0);
	// 颜色混合操作：fragColor乘以纹理图像的rgb通道，使其在顶点附近的颜色更单一化
	outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);

}