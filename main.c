// Project for TSBK07
// Andreas Pettersson & Jonas Ehn


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "simplefont.h"

#include "frustum.h"
#include "keyboard.h"
#include "heightMap.h"
#include "skybox.h"
#include "defines.h"
#include "draw.h"
#include "initFunctions.h"


// vertex array object
Model *terrainModel, *lakeModel, *plateModel;
// Reference to shader program
GLuint program, snowprogram, texprogram;
// Textures for terrain and lake height maps
TextureData terrainTexture, lakeTexture;
// References to different textures
GLuint grassTex;
GLuint skyTex;
GLuint dirtTex;
GLuint snowTex;
GLuint heightTex;
GLuint snowflakeTex;
GLuint bumpTex;


// References to the textures used in skybox
TextureData texData[6];


// Camera settings
const float cameraHeight = 15.0;
vec3 camPos = { 0.0f, 0.0f, 0.0f };
vec3 camLookAt = { 0.0f, 25.0f, 15.0f };
vec3 camUp = { 0.0f, 1.0f, 0.0f };

// Map settings
GLfloat scaling_factor = 20.0;

// Simulation settings
GLuint isSnowing = 1;
int simulationSpeed;
struct vec2int windDirection;






void init(void)
{
	// GL inits
	glClearColor(1, 0, 0, 1);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");


	// Load models
	plateModel = LoadModelPlus("./res/plate.obj");
	printError("GL init load models");

	// Build the environment cube
	LoadSkyboxData();

	// Load textures
	LoadTGATextureSimple("./res/cloudysunset.tga", &skyTex);
	LoadTGATextureSimple("./res/grassplus.tga", &grassTex);
	LoadTGATextureSimple("./res/dirt.tga", &dirtTex);
	LoadTGATextureSimple("./res/snowTexture.tga", &snowTex);
	LoadTGATextureSimple("./res/snowflake.tga", &snowflakeTex);
	LoadTGATextureSimple("./res/fft-terrain.tga", &heightTex);
	LoadTGATextureSimple("./res/conc2.tga", &bumpTex);
	printError("GL init load textures");

	// Load and compile shader
	program = loadShaders("shader.vert", "shader.frag");
	snowprogram = loadShaders("snow_shader.vert", "snow_shader.frag");
	texprogram = loadShaders("tex.vert", "tex.frag");
	printError("GL init load shader programs");


	// Projection matrix and frustum culling
	CreateAndSendProjectionMatrix(program, snowprogram, texprogram);
	printError("GL init create and send projectionMatrix");
	CreateAndSendFrustumCullingParameters(program, snowprogram);
	printError("GL init create and send frustum points and normals");


	// Load terrain data
	LoadTGATextureData("./res/fft-terrain.tga", &terrainTexture);
	terrainModel = GenerateTerrain(&terrainTexture);
	// Set a proper height for the camera
	camPos.y = GetHeight(&terrainTexture, camPos.x, camPos.z) + cameraHeight;

	// Load lake
	LoadTGATextureData("./res/lake_bottom.tga", &lakeTexture);
	lakeModel = GenerateTerrain(&lakeTexture);
	printError("GL init load terrain and lake");

	

	createSSBO();
	printError("GL init create and send buffer for GPU storage");

	// Bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, skyTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dirtTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, heightTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, snowTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, snowflakeTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, bumpTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	printError("GL init bind textures");


	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "heightTex"), heightTex);
	glUniform1i(glGetUniformLocation(snowprogram, "snowflakeTex"), snowflakeTex);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "snowTex"), snowTex);
	glUniform1i(glGetUniformLocation(program, "cubemap"), 6); // Since GL_TEXTURE6 is used for the cubemap
	glUniform1i(glGetUniformLocation(program, "bumpTex"), bumpTex);
	glUseProgram(texprogram);
	glUniform1i(glGetUniformLocation(texprogram, "tex"), 6); // Since GL_TEXTURE6 is used for the cubemap
	printError("GL init send texture unit numbers to shader");


	simulationSpeed = 100;
	windDirection.x = 0;
	windDirection.z = 0;
	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "simulationSpeed"), simulationSpeed);
	glUniform1i(glGetUniformLocation(snowprogram, "isSnowing"), isSnowing);
	glUniform1i(glGetUniformLocation(snowprogram, "x_wind"), windDirection.x);
	glUniform1i(glGetUniformLocation(snowprogram, "z_wind"), windDirection.z);
	printError("GL init send initial simulation values");


	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "imageScale"), imageScale);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "imageScale"), imageScale);
	printError("GL init send imageScale");



}







void display(void)
{
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printError("GL display clear screen");

	// Handle key events, i.e. movement
	handleKeyboardEvent();
	printError("GL display handle keyboard events");

	// Get the elapsed time
	GLfloat t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);



	// Build and send worldToView and camPos
	mat4 worldToView = lookAtv(camPos, camLookAt, camUp);
	glUseProgram(program);
	glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &camPos.x);
	glUniformMatrix4fv(glGetUniformLocation(program, "worldToViewMatrix"), 1, GL_TRUE, worldToView.m);
	glUseProgram(snowprogram);
	glUniform3fv(glGetUniformLocation(snowprogram, "cameraPos"), 1, &camPos.x);
	glUniformMatrix4fv(glGetUniformLocation(snowprogram, "worldToViewMatrix"), 1, GL_TRUE, worldToView.m);
	printError("GL display send camera and worldToView");


	

	// Draw world
	drawSkybox(worldToView, texData);
	drawLake(lakeModel, lakeTexture);
	drawTerrain(terrainModel);
	printError("GL display draw world");


	// Draw snow, and send time and modelToWorld before
	glUseProgram(program);
	glUniform1f(glGetUniformLocation(program, "time"), t);
	glUseProgram(snowprogram);
	glUniform1f(glGetUniformLocation(snowprogram, "time"), t);

	mat4 scaleMatrix = S((GLfloat) 0.1, (GLfloat) 0.1, (GLfloat) 0.1);
	glUniformMatrix4fv(glGetUniformLocation(snowprogram, "scaleMatrix"), 1, GL_TRUE, scaleMatrix.m);
	DrawModelInstanced(plateModel, snowprogram, "inPosition", NULL, "inTexCoord", no_particles);

	printError("GL display draw snow");

	// Draw some informative text on the screen	
	drawInformationText(simulationSpeed, windDirection);
	printError("GL display draw text");


	glutSwapBuffers();
	printError("GL display swap buffers");
}

void timer(int i)
{
	glutTimerFunc(20, &timer, i);
	glutPostRedisplay();
}

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	sfSetRasterSize(w, h);
}





int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(WIN_X_SIZE, WIN_Y_SIZE);
	glutCreateWindow ("TSBK07 Project");
	glutFullScreen();
#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(mouseMoved);
	glutKeyboardUpFunc(keyReleased);
	glutHideCursor();
	glutRepeatingTimer(20);
	init();
	loadSkyboxTextures(texData);
	sfMakeRasterFont();
	sfSetRasterSize(WIN_X_SIZE, WIN_Y_SIZE);
	glutMainLoop();
	exit(0);
}
