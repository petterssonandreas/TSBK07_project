// Project for TSBK07
// Andreas Pettersson & Jonas Ehn

#include "MicroGlut.h"
// uses framework Cocoa
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>

#define imageScale 2// How big the world is in terms of 256x256. 
#define no_particles 65536
#define WIN_X_SIZE 700
#define WIN_Y_SIZE 700

#ifdef WIN32
#include <windows.h>
#endif


void handleKeyboardEvent();
void draw(Model* model, mat4 mdlMatrix);
void drawLake(Model* model, mat4 mdlMatrix);
void drawSkybox();
void drawTerrain();
void DrawModelInstanced(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName, int count);
vec3 VectorReverse(vec3 a);


const float cameraHeight = 15.0;
vec3 camPos = { 10.0f, 20.0f, 0.0f };
vec3 camLookAt = { 0.0f, 0.0f, 0.0f };
vec3 camUp = { 0.0f, 1.0f, 0.0f };

GLuint isSnowing = 1;

GLfloat scaling_factor = 20.0;


float GetHeight(TextureData *tex, float x, float z)
{
	x = imageScale*x;
	z = imageScale*z;

	int x_floored = (int) floor(x);
	int z_floored = (int) floor(z);
	int x_ceiled = (int) ceil(x);
	int z_ceiled = (int) ceil(z);

	float height = 0;


	if ((x_floored >= 0 && x_ceiled < (int) tex->width) && (z_floored >= 0 && z_ceiled < (int) tex->height))
	{
		float x_floor_z_floor_height = tex->imageData[(x_floored + z_floored * tex->width) * (tex->bpp / 8)] / scaling_factor;
		float x_floor_z_ceil_height = tex->imageData[(x_floored + z_ceiled * tex->width) * (tex->bpp / 8)] / scaling_factor;
		float x_ceil_z_floor_height = tex->imageData[(x_ceiled + z_floored * tex->width) * (tex->bpp / 8)] / scaling_factor;
		float x_ceil_z_ceil_height = tex->imageData[(x_ceiled + z_ceiled * tex->width) * (tex->bpp / 8)] / scaling_factor;


		// Bottom triangle
		if (x - x_floored <= z - z_floored)
		{
			float x_incline = x_ceil_z_floor_height - x_floor_z_floor_height;
			float z_incline = x_floor_z_ceil_height - x_floor_z_floor_height;

			height = ((x - x_floored)*x_incline + (z - z_floored)*z_incline) + x_floor_z_floor_height;
		}
		// Top triangle
		else
		{
			float x_incline = x_ceil_z_ceil_height - x_floor_z_ceil_height;
			float z_incline = x_ceil_z_ceil_height - x_ceil_z_floor_height;

			height = x_ceil_z_ceil_height - ((x_ceiled - x)*x_incline + (z_ceiled - z)*z_incline);
		}

		//return tex->imageData[(x_int + z_int * tex->width) * (tex->bpp/8)] / scaling_factor;
	}

	// Lack of error handling...
	return height;
}


Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	unsigned int x, z;

	srand((unsigned int) time(NULL));

	GLfloat *vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = malloc(sizeof(GLuint) * triangleCount * 3);

	printf("bpp %d\n", tex->bpp);
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
			// Vertex array. You need to scale this properly
			vertexArray[(x + z * tex->width) * 3 + 0] = (GLfloat) x/imageScale;
			vertexArray[(x + z * tex->width) * 3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			vertexArray[(x + z * tex->width) * 3 + 2] = (GLfloat) z/imageScale;
			// Normal vectors. You need to calculate these.
			GLfloat x_prev_intensity = 0;
			GLfloat x_next_intensity = 0;
			GLfloat z_prev_intensity = 0;
			GLfloat z_next_intensity = 0;
			GLfloat intensity = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			if (x > 0)
			{
				x_prev_intensity = tex->imageData[((x - 1) + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (x < tex->width - 1)
			{
				x_next_intensity = tex->imageData[((x + 1) + z * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (z > 0)
			{
				z_prev_intensity = tex->imageData[(x + (z - 1) * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (z < tex->height - 1)
			{
				z_next_intensity = tex->imageData[(x + (z + 1) * tex->width) * (tex->bpp / 8)] / scaling_factor;
			}
			if (x == 0)
			{
				x_prev_intensity = intensity - (x_next_intensity - intensity);
			}
			else if (x == tex->width - 1)
			{
				x_next_intensity = intensity - (x_prev_intensity - intensity);
			}
			if (z == 0)
			{
				z_prev_intensity = intensity - (z_next_intensity - intensity);
			}
			else if (z == tex->height - 1)
			{
				z_next_intensity = intensity - (z_prev_intensity - intensity);
			}

			vec3 normal = { x_prev_intensity - x_next_intensity, 0.6, z_prev_intensity - z_next_intensity };
			normal = Normalize(normal);

			normalArray[(x + z * tex->width) * 3 + 0] = normal.x;
			normalArray[(x + z * tex->width) * 3 + 1] = normal.y;
			normalArray[(x + z * tex->width) * 3 + 2] = normal.z;
			// Texture coordinates. You may want to scale them.
			texCoordArray[(x + z * tex->width) * 2 + 0] = (GLfloat) x/100.f;
			texCoordArray[(x + z * tex->width) * 2 + 1] = (GLfloat) z/100.f;
		}
	for (x = 0; x < tex->width - 1; x++)
		for (z = 0; z < tex->height - 1; z++)
		{
			// Triangle 1
			indexArray[(x + z * (tex->width - 1)) * 6 + 0] = x + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 1] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 2] = x + 1 + z * tex->width;
			// Triangle 2
			indexArray[(x + z * (tex->width - 1)) * 6 + 3] = x + 1 + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 4] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 5] = x + 1 + (z + 1) * tex->width;
		}

	// End of terrain generation

	// Create Model and upload to GPU:

	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount * 3);

	printf("Triangle count: %d\n", triangleCount);

	return model;
}


// vertex array object
Model *terrainModel, *lakeModel, *skyModel, *plateModel;
// Reference to shader program
GLuint program, snowprogram;
// Textures for terrain and lake height maps
TextureData terrainTexture, lakeTexture;
// References to different textures
GLuint grassTex;
GLuint skyTex;
GLuint dirtTex;
GLuint snowTex;
GLuint heightTex;
GLuint snowflakeTex;

int simulationSpeed;


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

	// Snow texture
	//glGenTextures(1, &snowTexture);
	//glBindTexture(GL_TEXTURE_2D, snowTexture);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8UI, 256, 256);
	//glBindTexture(GL_TEXTURE_2D, 5);


	// Load models
	skyModel = LoadModelPlus("./res/skyboxsnow.obj");
	plateModel = LoadModelPlus("./res/plate.obj");
	printError("GL init load models");

	// Load textures
	LoadTGATextureSimple("./res/cloudysunset.tga", &skyTex);
	LoadTGATextureSimple("./res/grassplus.tga", &grassTex);
	LoadTGATextureSimple("./res/dirt.tga", &dirtTex);
	LoadTGATextureSimple("./res/snowTexture.tga", &snowTex);
	LoadTGATextureSimple("./res/snowflake.tga", &snowflakeTex);
	LoadTGATextureSimple("./res/fft-terrain.tga", &heightTex);
	printError("GL init load textures");

	// Load and compile shader
	program = loadShaders("shader.vert", "shader.frag");
	snowprogram = loadShaders("snow_shader.vert", "snow_shader.frag");
	printError("GL init load shader programs");

	// Create and send projectionMatrix
	float left = -0.1f;
	float right = 0.1f;
	float bottom = -0.1f;
	float top = 0.1f;
	float znear = 0.2f;
	float zfar = 1000.0f;

	float farAngle = (float) tan(right/znear);
	float farRight = zfar * (float) atan(farAngle);
	float farLeft = - farRight;
	float farTop = farRight;
	float farBottom = -farTop;

	mat4 projectionMatrix = frustum(left, right, bottom, top, znear, zfar);
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(snowprogram);
	glUniformMatrix4fv(glGetUniformLocation(snowprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	printError("GL init create and send projectionMatrix");

	vec3 ntl = {left, top, -znear};
	vec3 ntr = {right, top, -znear};
	vec3 nbl = {left, bottom, -znear};
	vec3 nbr = {right, bottom, -znear};
	vec3 ftl = {farLeft, farTop, -zfar};
	vec3 ftr = {farRight, farTop, -zfar};
	vec3 fbl = {farLeft, farBottom, -zfar};
	vec3 fbr = {farRight, farBottom, -zfar};

	vec3 farNormal = VectorReverse(CalcNormalVector(ftl, ftr, fbl));
	vec3 leftNormal = CalcNormalVector(ntl, nbl, ftl);
	vec3 rightNormal = VectorReverse(CalcNormalVector(ntr, nbr, fbr));
	vec3 topNormal = VectorReverse(CalcNormalVector(ntl, ntr, ftl));
	vec3 bottomNormal = CalcNormalVector(nbl, nbr, fbl);

	glUseProgram(snowprogram);
	glUniform3fv(glGetUniformLocation(snowprogram, "ntl"), 1, &ntl.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "nbr"), 1, &nbr.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "ftl"), 1, &ftl.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "fbr"), 1, &fbr.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "farNormal"), 1, &farNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "leftNormal"), 1, &leftNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "rightNormal"), 1, &rightNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "topNormal"), 1, &topNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "bottomNormal"), 1, &bottomNormal.x);
	glUseProgram(program);
	glUniform3fv(glGetUniformLocation(program, "ntl"), 1, &ntl.x);
	glUniform3fv(glGetUniformLocation(program, "nbr"), 1, &nbr.x);
	glUniform3fv(glGetUniformLocation(program, "ftl"), 1, &ftl.x);
	glUniform3fv(glGetUniformLocation(program, "fbr"), 1, &fbr.x);
	glUniform3fv(glGetUniformLocation(program, "farNormal"), 1, &farNormal.x);
	glUniform3fv(glGetUniformLocation(program, "leftNormal"), 1, &leftNormal.x);
	glUniform3fv(glGetUniformLocation(program, "rightNormal"), 1, &rightNormal.x);
	glUniform3fv(glGetUniformLocation(program, "topNormal"), 1, &topNormal.x);
	glUniform3fv(glGetUniformLocation(program, "bottomNormal"), 1, &bottomNormal.x);
	printError("GL init create and send frustum points and normals");


	// Load terrain data
	LoadTGATextureData("./res/fft-terrain.tga", &terrainTexture);
	terrainModel = GenerateTerrain(&terrainTexture);
	// Load lake
	LoadTGATextureData("./res/lake_bottom.tga", &lakeTexture);
	lakeModel = GenerateTerrain(&lakeTexture);
	printError("GL init load terrain and lake");

	GLuint ssbo = 0;
	
	static struct ssbo_data_t
	{
		GLuint snow[2*256*2*256];
		vec3 position[no_particles];
	} ssbo_data;

	for (int j = 1; j < 2*256*2*256; j++)
	{
		ssbo_data.snow[j] = 0;
	}
	for (int i = 1; i < no_particles; i++)
	{
		//ssbo_data.snow[i] = 0;
		ssbo_data.position[i].x = 0;
		ssbo_data.position[i].y = 0;
		ssbo_data.position[i].z = 0;
	}
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (sizeof(struct ssbo_data_t)), NULL, GL_DYNAMIC_COPY);
	//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ssbo_data), &ssbo_data);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	printError("GL init create and send buffer for GPU storage");
	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "imageScale"), imageScale);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "imageScale"), imageScale);

	// Set a proper height for the camera
	camPos.y = GetHeight(&terrainTexture, camPos.x, camPos.z) + 5;
	camLookAt.y = camPos.y;

	// Bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, skyTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dirtTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, heightTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, snowTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, snowflakeTex);
	printError("GL init bind textures");


	simulationSpeed = 100;

	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "heightTex"), 3);
	glUniform1i(glGetUniformLocation(snowprogram, "snowflakeTex"), 5);
	glUniform1i(glGetUniformLocation(snowprogram, "simulationSpeed"), simulationSpeed);
	glUniform1i(glGetUniformLocation(snowprogram, "isSnowing"), isSnowing);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "snowTex"), 4);

	printError("GL init send texture unit numbers to shader");
}


bool firstCall = true;

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

	if (firstCall)
	{
		glUseProgram(program);
		glUniform1f(glGetUniformLocation(program, "time_0"), t);
		glUseProgram(snowprogram);
		glUniform1f(glGetUniformLocation(snowprogram, "time_0"), t);
		firstCall = false;
	}

	// Snow height
	//glDepthMask(false);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glBindImageTexture(4, snowTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI);

	// Build and send worldToView and camPos
	mat4 worldToView = lookAtv(camPos, camLookAt, camUp);
	glUseProgram(program);
	glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &camPos.x);
	glUniformMatrix4fv(glGetUniformLocation(program, "worldToViewMatrix"), 1, GL_TRUE, worldToView.m);
	glUseProgram(snowprogram);
	glUniformMatrix4fv(glGetUniformLocation(snowprogram, "worldToViewMatrix"), 1, GL_TRUE, worldToView.m);
	printError("GL display send camera and worldToView");


	// Draw world
	glUseProgram(program);
	drawSkybox();
	drawLake(lakeModel, T(0, -GetHeight(&lakeTexture, 0, 0), 0));
	drawTerrain();
	printError("GL display draw world");


	// Draw snow, and send time and modelToWorld before
	glUseProgram(snowprogram);
	glUniform1f(glGetUniformLocation(snowprogram, "time"), t);
	mat4 scaleMatrix = S((GLfloat) 0.05, (GLfloat) 0.05, (GLfloat) 0.05);
	glUniformMatrix4fv(glGetUniformLocation(snowprogram, "scaleMatrix"), 1, GL_TRUE, scaleMatrix.m);
	DrawModelInstanced(plateModel, snowprogram, "inPosition", NULL, "inTexCoord", no_particles);

	printError("GL display draw snow");

	glutSwapBuffers();
	printError("GL display swap buffers");

	GLfloat t_current = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
	GLfloat fps = 1/((t_current - t)/1000);
	printf("fps: %4.2f \n", fps);
}

void timer(int i)
{
	glutTimerFunc(20, &timer, i);
	glutPostRedisplay();
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

#define MAX_SIM_SPEED 100000
#define MIN_SIM_SPEED 1

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

	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "simulationSpeed"), simulationSpeed);
	glUniform1i(glGetUniformLocation(snowprogram, "isSnowing"), isSnowing);
	printf("simulationSpeed: %d\n", simulationSpeed);
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(WIN_X_SIZE, WIN_Y_SIZE);
	glutCreateWindow ("TSBK07 Project");
#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display);
	glutPassiveMotionFunc(mouseMoved);
	glutKeyboardUpFunc(keyReleased);
	glutHideCursor();
	glutRepeatingTimer(20);
	init();
	glutMainLoop();
	exit(0);
}



void draw(Model* model, mat4 mdlMatrix)
{
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, mdlMatrix.m);
	DrawModel(model, program, "inPosition", "inNormal", "inTexCoord");
}

void drawLake(Model* model, mat4 mdlMatrix)
{
	glUniform1i(glGetUniformLocation(program, "drawing_lake_bottom"), 1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, mdlMatrix.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	DrawModel(model, program, "inPosition", "inNormal", "inTexCoord");
	glUniform1i(glGetUniformLocation(program, "drawing_lake_bottom"), 0);
}

void drawSkybox()
{
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(program, "drawing_skyBox"), 1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, T((GLfloat) camPos.x, (GLfloat) (camPos.y - 0.2), (GLfloat) camPos.z).m);
	glUniform1i(glGetUniformLocation(program, "tex"), 1);
	DrawModel(skyModel, program, "inPosition", "inNormal", "inTexCoord");
	glUniform1i(glGetUniformLocation(program, "drawing_skyBox"), 0);
	glEnable(GL_DEPTH_TEST);
}

void drawTerrain()
{
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, IdentityMatrix().m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glUniform1i(glGetUniformLocation(program, "dirtTex"), 2);
	DrawModel(terrainModel, program, "inPosition", "inNormal", "inTexCoord"); // Draw terrain
}

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

void DrawModelInstanced(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName, int count)
{
	if (m != NULL)
	{
		GLint loc;

		glBindVertexArray(m->vao);	// Select VAO
		glBindBuffer(GL_ARRAY_BUFFER, m->vb);
		loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(loc);
		}
		else
			fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", vertexVariableName);

		if (normalVariableName != NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", normalVariableName);
		}

		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL) && (texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", texCoordVariableName);
		}

		glDrawElementsInstanced(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_INT, 0L, count);
	}
}

vec3 VectorReverse(vec3 a)
{
	vec3 zerovec = {0,0,0};
	return VectorSub(zerovec, a);
}