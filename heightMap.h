#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H



#include "MicroGlut.h"
#include "LoadTGA.h"
#include "loadobj.h"

#define imageScale 2 // How big the world is in terms of 256x256.

extern GLfloat scaling_factor;
//#define scaling_factor 20

extern TextureData terrainTexture;

float GetHeight(TextureData *tex, float x, float z);
Model* GenerateTerrain(TextureData *tex);


#endif