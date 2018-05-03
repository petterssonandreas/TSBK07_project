
#include <math.h>
#include <time.h>

#include "heightMap.h"
#include "LoadTGA.h"
#include "loadobj.h"
#include "VectorUtils3.h"


float GetHeight(TextureData *tex, float x, float z)
{
	x = imageScale * x;
	z = imageScale * z;

	int x_floored = (int) floor(x);
	int z_floored = (int) floor(z);
	int x_ceiled = (int) ceil(x);
	int z_ceiled = (int) ceil(z);

	float height = 0;


	if ((x_floored >= 0 && x_ceiled < (int) tex->width) && (z_floored >= 0 && z_ceiled < (int) tex->height))
	{
		float x_floor_z_floor_height = tex->imageData[(x_floored + z_floored * tex->width) * (tex->bpp / 8)] / scaling_factor;
		float x_floor_z_ceil_height = tex->imageData[(x_floored + z_ceiled * tex->width) * (tex->bpp / 8)] / scaling_factor;
		float x_ceil_z_floor_height = tex->imageData[(x_ceiled + z_floored * tex->width) * (tex->bpp / 8)] / scaling_factor;
		float x_ceil_z_ceil_height = tex->imageData[(x_ceiled + z_ceiled * tex->width) * (tex->bpp / 8)] / scaling_factor;


		// Bottom triangle
		if (x - x_floored <= z - z_floored)
		{
			float x_incline = x_ceil_z_floor_height - x_floor_z_floor_height;
			float z_incline = x_floor_z_ceil_height - x_floor_z_floor_height;

			height = ((x - x_floored)*x_incline + (z - z_floored)*z_incline) + x_floor_z_floor_height;
		}
		// Top triangle
		else
		{
			float x_incline = x_ceil_z_ceil_height - x_floor_z_ceil_height;
			float z_incline = x_ceil_z_ceil_height - x_ceil_z_floor_height;

			height = x_ceil_z_ceil_height - ((x_ceiled - x)*x_incline + (z_ceiled - z)*z_incline);
		}

		//return tex->imageData[(x_int + z_int * tex->width) * (tex->bpp/8)] / scaling_factor;
	}

	// Lack of error handling...
	return height;
}



Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	unsigned int x, z;

	GLfloat *vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = malloc(sizeof(GLuint) * triangleCount * 3);

	printf("bpp %d\n", tex->bpp);
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
			// Vertex array. You need to scale this properly
			vertexArray[(x + z * tex->width) * 3 + 0] = (GLfloat) x/imageScale;
			vertexArray[(x + z * tex->width) * 3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			vertexArray[(x + z * tex->width) * 3 + 2] = (GLfloat) z/imageScale;
			// Normal vectors. You need to calculate these.
			GLfloat x_prev_intensity = 0;
			GLfloat x_next_intensity = 0;
			GLfloat z_prev_intensity = 0;
			GLfloat z_next_intensity = 0;
			GLfloat intensity = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			if (x > 0)
			{
				x_prev_intensity = tex->imageData[((x - 1) + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (x < tex->width - 1)
			{
				x_next_intensity = tex->imageData[((x + 1) + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (z > 0)
			{
				z_prev_intensity = tex->imageData[(x + (z - 1) * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (z < tex->height - 1)
			{
				z_next_intensity = tex->imageData[(x + (z + 1) * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (x == 0)
			{
				x_prev_intensity = intensity - (x_next_intensity - intensity);
			}
			else if (x == tex->width - 1)
			{
				x_next_intensity = intensity - (x_prev_intensity - intensity);
			}
			if (z == 0)
			{
				z_prev_intensity = intensity - (z_next_intensity - intensity);
			}
			else if (z == tex->height - 1)
			{
				z_next_intensity = intensity - (z_prev_intensity - intensity);
			}

			vec3 normal = { x_prev_intensity - x_next_intensity, (GLfloat) 0.6, z_prev_intensity - z_next_intensity };
			normal = Normalize(normal);

			normalArray[(x + z * tex->width) * 3 + 0] = normal.x;
			normalArray[(x + z * tex->width) * 3 + 1] = normal.y;
			normalArray[(x + z * tex->width) * 3 + 2] = normal.z;
			// Texture coordinates. You may want to scale them.
			texCoordArray[(x + z * tex->width) * 2 + 0] = (GLfloat) x/100.f;
			texCoordArray[(x + z * tex->width) * 2 + 1] = (GLfloat) z/100.f;
		}
	for (x = 0; x < tex->width - 1; x++)
		for (z = 0; z < tex->height - 1; z++)
		{
			// Triangle 1
			indexArray[(x + z * (tex->width - 1)) * 6 + 0] = x + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 1] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 2] = x + 1 + z * tex->width;
			// Triangle 2
			indexArray[(x + z * (tex->width - 1)) * 6 + 3] = x + 1 + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 4] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 5] = x + 1 + (z + 1) * tex->width;
		}

	// End of terrain generation

	// Create Model and upload to GPU:

	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount * 3);

	printf("Triangle count: %d\n", triangleCount);

	return model;
}