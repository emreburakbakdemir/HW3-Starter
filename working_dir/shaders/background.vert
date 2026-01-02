#version 430

#define IN_POS layout(location = 0)
#define IN_UV layout(location = 2)

#define U_TRANSFORM_MODEL layout(location = 0)
#define U_TRANSFORM_VIEW layout(location = 1)
#define U_TRANSFORM_PROJ layout(location = 2)

in IN_POS vec3 vPos;
in IN_UV vec2 vUV;

out vec2 fUV;
out gl_PerVertex {vec4 gl_Position;};

U_TRANSFORM_MODEL uniform mat4 uModel;
U_TRANSFORM_VIEW uniform mat4 uView;
U_TRANSFORM_PROJ uniform mat4 uProjection;

void main(void)
{
	fUV = vUV;
	vec4 pos = uProjection * uView * uModel * vec4(vPos, 1.0);
	gl_Position = pos;
}
