// tex.vert
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A vertex shader that only handles the skybox

#version 450

// ----------- IN -------------
in  vec3 inPosition;
in  vec2 inTexCoord;

// ----------- OUT ------------
out vec2 texCoord;

// ----------- UNIFORMS -------
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;

void main(void)
{
	texCoord = inTexCoord;
	gl_Position = projMatrix * worldToViewMatrix * vec4(inPosition, 1.0);
}
