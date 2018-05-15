// skybox.c
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A file that creates the skybox.

#include "skybox.h"
#include "MicroGlut.h"
#include "loadobj.h"
#include <stdio.h>

// The vertices for the environment box
GLfloat vertices[6][4*3] =
{
	{ // +x
		0.5,-0.5,-0.5,
		0.5,0.5,-0.5,
		0.5,0.5,0.5,
		0.5,-0.5,0.5,
	},
	{ // -x
		-0.5,-0.5,-0.5,
		-0.5,-0.5,0.5,
		-0.5,0.5,0.5,
		-0.5,0.5,-0.5,
	},
	{ // +y
		0.5,0.5,-0.5,
		-0.5,0.5,-0.5,
		-0.5,0.5,0.5,
		0.5,0.5,0.5,
	},
	{ // -y
		-0.5,-0.5,-0.5,
		0.5,-0.5,-0.5,
		0.5,-0.5,0.5,
		-0.5,-0.5,0.5
	},
	{ // +z
		-0.5,-0.5,0.5,
		0.5,-0.5,0.5,
		0.5,0.5,0.5,
		-0.5,0.5,0.5,
	},
	{ // -z
		-0.5,-0.5,-0.5,
		-0.5,0.5,-0.5,
		0.5,0.5,-0.5,
		0.5,-0.5,-0.5,
	}
};

// Texture coordinates for the environment box
GLfloat texcoord[6][4*2] =
{
	{
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0,
	},
	{
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
	},
	{
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	},
	{
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
	},
	{
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
	},
	{
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0,
	}
};
GLuint indices[6][6] =
{
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2}
};

Model *box[6];

void LoadSkyboxData()
{
	// Build the environment cube
	for (int i = 0; i < 6; i++)
	{
		box[i] = LoadDataToModel(
			vertices[i],
			NULL,
			texcoord[i],
			NULL,
			indices[i],
			4,
			6);
	}
}

