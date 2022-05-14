#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec3 outLight;

out gl_PerVertex {
	vec4 gl_Position;
};

layout(push_constant) uniform PushConsts {
	mat4 mvp;
	mat4 normalMVP;
	vec3 lightVector;
} pushConsts;

void main()
{
	mat4 mvp_mat = pushConsts.mvp;
	mat4 normal_mat = pushConsts.normalMVP;
	vec3 light_vec = pushConsts.lightVector;

	gl_Position = mvp_mat * vec4(inPos.xyz, 1.0);
	outNormal = normalize( normal_mat * vec4(inNormal, 0.0) );
    outUV = inUV;
	outLight = light_vec;
}
