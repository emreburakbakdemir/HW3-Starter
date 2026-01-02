#version 430
/*
	File Name	: shadow.frag
	Description	: Shadow pass fragment shader
*/

#define OUT_FBO	layout(location = 0)

// Input
in float fDepth;

// Output
out OUT_FBO float fboDepth;

void main(void)
{
	// Write depth value to color attachment
	fboDepth = fDepth;
}
