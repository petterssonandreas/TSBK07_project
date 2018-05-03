
#include "keyboard.h"
#include "VectorUtils3.h"
#include "MicroGlut.h"
#include "heightMap.h"
#include "defines.h"

#ifdef WIN32
#include <windows.h>
#endif


void handleKeyboardEvent()
{
#ifdef WIN32
	if (glutKeyIsDown(27)) // ESC key
	{
		// Send exit message to windows
		PostQuitMessage(0);
	}
#endif

	if (glutKeyIsDown('w') || glutKeyIsDown('W'))
	{
		vec3 stepDirection = VectorSub(camLookAt, camPos);
		stepDirection.y = 0; // Set to zero to step in xz-plane
		stepDirection = Normalize(stepDirection);
		camLookAt = VectorAdd(camLookAt, stepDirection);
		camPos = VectorAdd(camPos, stepDirection);
		float camPos_y = camPos.y;
		camPos.y = GetHeight(&terrainTexture, camPos.x, camPos.z) + cameraHeight;
		camLookAt.y = camLookAt.y + (camPos.y - camPos_y);
	}

	if (glutKeyIsDown('s') || glutKeyIsDown('S'))
	{
		vec3 stepDirection = VectorSub(camLookAt, camPos);
		stepDirection.y = 0; // Set to zero to step in xz-plane
		stepDirection = Normalize(stepDirection);
		camLookAt = VectorSub(camLookAt, stepDirection);
		camPos = VectorSub(camPos, stepDirection);
		float camPos_y = camPos.y;
		camPos.y = GetHeight(&terrainTexture, camPos.x, camPos.z) + cameraHeight;
		camLookAt.y = camLookAt.y + (camPos.y - camPos_y);
	}

	if (glutKeyIsDown('d') || glutKeyIsDown('D'))
	{
		vec3 stepDirection = CrossProduct(VectorSub(camLookAt, camPos), camUp);
		stepDirection.y = 0; // Set to zero to step in xz-plane
		stepDirection = Normalize(stepDirection);
		camLookAt = VectorAdd(camLookAt, stepDirection);
		camPos = VectorAdd(camPos, stepDirection);
		float camPos_y = camPos.y;
		camPos.y = GetHeight(&terrainTexture, camPos.x, camPos.z) + cameraHeight;
		camLookAt.y = camLookAt.y + (camPos.y - camPos_y);
	}

	if (glutKeyIsDown('a') || glutKeyIsDown('A'))
	{
		vec3 stepDirection = CrossProduct(VectorSub(camLookAt, camPos), camUp);
		stepDirection.y = 0; // Set to zero to step in xz-plane
		stepDirection = Normalize(stepDirection);
		camLookAt = VectorSub(camLookAt, stepDirection);
		camPos = VectorSub(camPos, stepDirection);
		float camPos_y = camPos.y;
		camPos.y = GetHeight(&terrainTexture, camPos.x, camPos.z) + cameraHeight;
		camLookAt.y = camLookAt.y + (camPos.y - camPos_y);
	}

	if (glutKeyIsDown('q') || glutKeyIsDown('Q'))
	{
		vec3 stepDirection = VectorSub(camLookAt, camPos);
		camLookAt = MultMat3Vec3(mat4tomat3(ArbRotate(camUp, (GLfloat) 0.05)), stepDirection);
		camLookAt = VectorAdd(camPos, camLookAt);
	}

	if (glutKeyIsDown('e') || glutKeyIsDown('E'))
	{
		vec3 stepDirection = VectorSub(camLookAt, camPos);
		camLookAt = MultMat3Vec3(mat4tomat3(ArbRotate(camUp, (GLfloat) -0.05)), stepDirection);
		camLookAt = VectorAdd(camPos, camLookAt);
	}

	/*if (glutKeyIsDown('r'))
	{
	camLookAt = VectorAdd(camLookAt, Normalize(camUp));
	camPos = VectorAdd(camPos, Normalize(camUp));
	}

	if (glutKeyIsDown('f'))
	{
	camLookAt = VectorSub(camLookAt, Normalize(camUp));
	camPos = VectorSub(camPos, Normalize(camUp));
	}*/

	if (glutKeyIsDown('t') || glutKeyIsDown('T'))
	{
		vec3 stepDirection = VectorSub(camLookAt, camPos);
		vec3 planarComp = CrossProduct(stepDirection, camUp);

		camLookAt = MultMat3Vec3(mat4tomat3(ArbRotate(planarComp, (GLfloat) 0.05)), stepDirection);
		camLookAt = VectorAdd(camPos, camLookAt);

		camUp = Normalize(CrossProduct(planarComp, VectorSub(camLookAt, camPos)));
	}

	if (glutKeyIsDown('g') || glutKeyIsDown('G'))
	{
		vec3 stepDirection = VectorSub(camLookAt, camPos);
		vec3 planarComp = CrossProduct(stepDirection, camUp);

		camLookAt = MultMat3Vec3(mat4tomat3(ArbRotate(planarComp, (GLfloat) -0.05)), stepDirection);
		camLookAt = VectorAdd(camPos, camLookAt);

		camUp = Normalize(CrossProduct(planarComp, VectorSub(camLookAt, camPos)));
	}
}

void mouseMoved(int x, int y)
{
	int xDiff = (WIN_X_SIZE / 2) - x;
	int yDiff = (WIN_Y_SIZE / 2) - y;

	GLfloat xAngle = (GLfloat) (xDiff / 600.0);
	GLfloat yAngle = (GLfloat) (yDiff / 600.0);

	// Look left, right
	vec3 stepDirection = VectorSub(camLookAt, camPos);
	camLookAt = MultMat3Vec3(mat4tomat3(ArbRotate(camUp, xAngle)), stepDirection);
	camLookAt = VectorAdd(camPos, camLookAt);

	// Look up, down
	stepDirection = VectorSub(camLookAt, camPos);
	vec3 planarComp = CrossProduct(stepDirection, camUp);
	camLookAt = MultMat3Vec3(mat4tomat3(ArbRotate(planarComp, yAngle)), stepDirection);
	camLookAt = VectorAdd(camPos, camLookAt);

	// Move cursor to center of window
	glutWarpPointer(WIN_X_SIZE / 2, WIN_Y_SIZE / 2);
}