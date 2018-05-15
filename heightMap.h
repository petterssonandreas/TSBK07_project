// heightMap.h
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// The functions that generate the terrain from an TGA-image

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H



#include "MicroGlut.h"
#include "LoadTGA.h"
#include "loadobj.h"

#define imageScale 2 // How big the world is in terms of 256x256.

extern GLfloat scaling_factor;

extern TextureData terrainTexture;

float GetHeight(TextureData *tex, float x, float z);
Model* GenerateTerrain(TextureData *tex);


#endif