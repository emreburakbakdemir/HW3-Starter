#version 430

#define OUT_FBO layout(location = 0)
#define T_TEXTURE layout(binding = 0)

in vec2 fUV;
out OUT_FBO vec4 fboColor;

uniform T_TEXTURE sampler2D tTexture;

void main(void)
{
	vec4 texColor = texture(tTexture, fUV);
	
	// Discard transparent pixels
	if (texColor.a < 0.1)
		discard;
	
	// Sun is emissive - output bright color
	fboColor = vec4(texColor.rgb * 1.5, texColor.a);
}
