#version 430

#define OUT_FBO layout(location = 0)
#define T_TEXTURE layout(binding = 0)

in vec3 fTexCoord;
out OUT_FBO vec4 fboColor;

uniform T_TEXTURE sampler2D tTexture;

void main(void)
{
	// Convert 3D direction to spherical UV coordinates for equirectangular texture
	vec3 dir = normalize(fTexCoord);
	
	// Spherical to UV mapping
	float u = 0.5 + atan(dir.z, dir.x) / (2.0 * 3.14159265359);
	float v = 0.5 - asin(clamp(dir.y, -1.0, 1.0)) / 3.14159265359;
	
	vec3 color = texture(tTexture, vec2(u, v)).rgb;
	fboColor = vec4(color, 1.0);
}
