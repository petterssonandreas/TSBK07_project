// draw.h
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// This file contains the function calls that draws objects in the scene

#ifndef DRAW_H
#define DRAW_H


#include "loadobj.h"
#include "VectorUtils3.h"
#include "LoadTGA.h"

void setShaderPrograms(GLuint program_, GLuint snowprogram_, GLuint texprogram_);

void drawInformationText(int simulationSpeed, struct vec2int windDirection);
void draw(Model* model, mat4 mdlMatrix);
void drawLake(Model* lakeModel, TextureData lakeTexture);
void drawSkybox(mat4 worldToView, TextureData * texData);
void drawTerrain(Model * terrainModel);
void DrawModelInstanced(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName, int count);


#endif