// tex.frag
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A fragment shader that handles the skybox only.

#version 450

// ----------- IN -------------
in vec2 texCoord;

// ----------- OUT ------------
out vec4 outColor;

// ----------- UNIFORMS -------
uniform sampler2D tex;

void main(void)
{
	outColor = texture(tex, texCoord);
}
