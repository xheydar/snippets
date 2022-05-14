#version 450

//shader input
layout (location = 0) in vec2 inUV;
//layout (location = 1) in vec4 inNormal;
//layout (location = 2) in vec3 inLight;

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
	//float intensity = dot( inNormal, vec4(inLight,0.0) );

	outColor = texture(texSampler, inUV);
	//outColor.rgb = outColor.rgb * intensity;
}
