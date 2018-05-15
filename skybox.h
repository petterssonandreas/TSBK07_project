// skybox.h
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A file that creates the skybox.

#ifndef SKYBOX_H
#define SKYBOX_H

#include "MicroGlut.h"
#include "loadobj.h"

extern Model *box[6];

void LoadSkyboxData();

#endif