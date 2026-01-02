#version 430
/*
	File Name	: cloud.frag
	Description	: Cloud fragment shader with alpha blending
*/

// Definitions
#define IN_UV			layout(location = 0)
#define IN_NORMAL		layout(location = 1)
#define IN_WORLD_POS	layout(location = 2)

#define OUT_FBO			layout(location = 0)

#define T_CLOUD			layout(binding = 0)

#define U_LIGHT_DIR		layout(location = 4)
#define U_LIGHT_COLOR	layout(location = 5)

// Input
in IN_UV		 vec2 fUV;
in IN_NORMAL	 vec3 fNormal;
in IN_WORLD_POS	 vec3 fWorldPos;

// Output
out OUT_FBO vec4 fboColor;

// Uniforms
U_LIGHT_DIR		uniform vec3 uLightDir;
U_LIGHT_COLOR	uniform vec3 uLightColor;

// Textures
uniform T_CLOUD sampler2D tCloud;

void main(void)
{
	// Sample cloud texture (alpha channel contains cloud opacity)
	vec4 cloudSample = texture(tCloud, fUV);

	// Normalize interpolated normal
	vec3 N = normalize(fNormal);

	// Light direction
	vec3 L = normalize(-uLightDir);

	// Simple diffuse lighting for clouds
	float diffuseTerm = max(dot(N, L), 0.0);
	vec3 cloudColor = vec3(1.0) * (0.3 + 0.7 * diffuseTerm) * uLightColor;

	// Output with alpha
	fboColor = vec4(cloudColor, cloudSample.a);
}
