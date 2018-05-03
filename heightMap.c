
#include <math.h>

#include "heightMap.h"
#include "LoadTGA.h"


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