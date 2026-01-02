#version 430

#define OUT_FBO layout(location = 0)
#define T_TEXTURE layout(binding = 0)

in vec3 fTexCoord;
out OUT_FBO vec4 fboColor;

uniform T_TEXTURE sampler2D tTexture;

void main(void)
{
	// DEBUG: Output solid blue to verify background is rendering
	fboColor = vec4(0.0, 0.0, 1.0, 1.0);
	return;

	// Convert 3D position to spherical UV coordinates
	vec3 dir = normalize(fTexCoord);
	float u = 0.5 + atan(dir.z, dir.x) / (2.0 * 3.14159265359);
	float v = 0.5 - asin(dir.y) / 3.14159265359;
	
	vec3 color = texture(tTexture, vec2(u, v)).rgb;
	fboColor = vec4(color, 1.0);
}
