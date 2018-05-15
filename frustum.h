// frustum.h
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// This file contains the calls that involves the frustum

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "MicroGlut.h"
#include "VectorUtils3.h"

vec3 VectorReverse(vec3 a);
void CreateAndSendProjectionMatrix(GLuint program, GLuint snowprogram, GLuint texprogram);
void CreateAndSendFrustumCullingParameters(GLuint program, GLuint snowprogram);


#endif