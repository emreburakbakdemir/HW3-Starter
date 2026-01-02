#version 430
/*
	File Name	: shadow.vert
	Description	: Shadow pass vertex shader
*/

#define IN_POS			layout(location = 0)

#define U_TRANSFORM_MODEL	layout(location = 0)
#define U_LIGHT_VP			layout(location = 7)

// Input
in IN_POS vec3 vPos;

// Output
out gl_PerVertex {vec4 gl_Position;};
out float fDepth;

// Uniforms
U_TRANSFORM_MODEL	uniform mat4 uModel;
U_LIGHT_VP			uniform mat4 uLightVP;

void main(void)
{
	vec4 worldPos = uModel * vec4(vPos, 1.0);
	vec4 lightSpacePos = uLightVP * worldPos;
	
	gl_Position = lightSpacePos;
	fDepth = lightSpacePos.z;
}
