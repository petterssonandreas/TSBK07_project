#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "VectorUtils3.h"

extern vec3 camPos;
extern vec3 camLookAt;
extern vec3 camUp;

extern const float cameraHeight;

void handleKeyboardEvent();
void mouseMoved(int x, int y);


#endif