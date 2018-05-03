#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "VectorUtils3.h"
#include "defines.h"

#define MAX_SIM_SPEED 65536
#define MAX_WIND_SPEED 256
#define MIN_SIM_SPEED 1

extern vec3 camPos;
extern vec3 camLookAt;
extern vec3 camUp;

extern const float cameraHeight;

extern int simulationSpeed;
extern int isSnowing;
extern struct vec2int windDirection;

extern GLuint program;
extern GLuint snowprogram;

void handleKeyboardEvent();
void mouseMoved(int x, int y);
void keyReleased(unsigned char key, int x, int y);


#endif