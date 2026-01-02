#version 430

#define IN_POS layout(location = 0)

#define U_TRANSFORM_MODEL layout(location = 0)
#define U_TRANSFORM_VIEW layout(location = 1)
#define U_TRANSFORM_PROJ layout(location = 2)

in IN_POS vec3 vPos;

out vec3 fTexCoord;
out gl_PerVertex {vec4 gl_Position;};

U_TRANSFORM_MODEL uniform mat4 uModel;
U_TRANSFORM_VIEW uniform mat4 uView;
U_TRANSFORM_PROJ uniform mat4 uProjection;

void main(void)
{
	// Pass vertex position as texture coordinate for spherical mapping
	fTexCoord = vPos;
	
	// Remove translation from view matrix to keep background centered on camera
	mat4 viewNoTranslate = mat4(mat3(uView));
	
	vec4 pos = uProjection * viewNoTranslate * vec4(vPos, 1.0);
	gl_Position = pos.xyww;  // z = w means depth = 1.0 (far plane)
}
