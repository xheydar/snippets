#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec3 inLight;

//layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
	float intensity = dot( inNormal, vec4(inLight,0.0) );

	outColor = vec4(inColor, 1.0);
	outColor.rgb = outColor.rgb * intensity;
}
