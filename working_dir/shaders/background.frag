#version 430

#define OUT_FBO layout(location = 0)
#define T_TEXTURE layout(binding = 0)

in vec2 fUV;
out OUT_FBO vec4 fboColor;

uniform T_TEXTURE sampler2D tTexture;

void main(void)
{
	vec3 color = texture(tTexture, fUV).rgb;
	fboColor = vec4(color, 1.0);
}
