
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
	if (glutKeyIsDown(27)) // ESC key
	{
#ifdef WIN32
		// Send exit message to windows
		PostQuitMessage(0);
#else
        // Hard exit if not on windows
        exit(0);
#endif
	}

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



void keyReleased(unsigned char key, int x, int y)
{
	if (key == '+')
	{
		simulationSpeed *= 2;
		if (simulationSpeed > MAX_SIM_SPEED)
		{
			simulationSpeed = MAX_SIM_SPEED;
		}
	}
	else if (key == '-')
	{
		simulationSpeed /= 2;
		if (simulationSpeed < MIN_SIM_SPEED)
		{
			simulationSpeed = MIN_SIM_SPEED;
		}
	}
	else if (key == ' ')
	{
		if (isSnowing == 1)
		{
			isSnowing = 0;
		}
		else
		{
			isSnowing = 1;
		}
	}
	else if (key == '0')
	{
		windDirection.x = 0;
		windDirection.z = 0;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		if (windDirection.x > 1)
		{
			windDirection.x /= 2;
		}
		else if (windDirection.x == 1)
		{
			windDirection.x = 0;
		}
		else if (windDirection.x == 0)
		{
			windDirection.x = -1;
		}
		else
		{
			windDirection.x *= 2;
		}
	}
	else if (key == GLUT_KEY_LEFT)
	{
		if (windDirection.x < -1)
		{
			windDirection.x /= 2;
		}
		else if (windDirection.x == -1)
		{
			windDirection.x = 0;
		}
		else if (windDirection.x == 0)
		{
			windDirection.x = 1;
		}
		else
		{
			windDirection.x *= 2;
		}
	}
	else if (key == GLUT_KEY_DOWN)
	{
		if (windDirection.z > 1)
		{
			windDirection.z /= 2;
		}
		else if (windDirection.x == 1)
		{
			windDirection.z = 0;
		}
		else if (windDirection.z == 0)
		{
			windDirection.z = -1;
		}
		else
		{
			windDirection.z *= 2;
		}
	}
	else if (key == GLUT_KEY_UP)
	{
		if (windDirection.z < -1)
		{
			windDirection.z /= 2;
		}
		else if (windDirection.z == -1)
		{
			windDirection.z = 0;
		}
		else if (windDirection.z == 0)
		{
			windDirection.z = 1;
		}
		else
		{
			windDirection.z *= 2;
		}
	}


	if (windDirection.x > MAX_WIND_SPEED)
	{
		windDirection.x = MAX_WIND_SPEED;
	}
	else if (windDirection.x < -MAX_WIND_SPEED)
	{
		windDirection.x = -MAX_WIND_SPEED;
	}

	if (windDirection.z > MAX_WIND_SPEED)
	{
		windDirection.z = MAX_WIND_SPEED;
	}
	else if (windDirection.z < -MAX_WIND_SPEED)
	{
		windDirection.z = -MAX_WIND_SPEED;
	}


	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "simulationSpeed"), simulationSpeed);
	glUniform1i(glGetUniformLocation(snowprogram, "isSnowing"), isSnowing);
	glUniform1i(glGetUniformLocation(snowprogram, "x_wind"), windDirection.x);
	glUniform1i(glGetUniformLocation(snowprogram, "z_wind"), windDirection.z);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "isSnowing"), isSnowing);
	printf("simulationSpeed: %d\n", simulationSpeed);
}
