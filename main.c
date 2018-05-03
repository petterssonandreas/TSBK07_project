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
// uses framework Cocoa
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

#define imageScale 2// How big the world is in terms of 256x256. 
#define no_particles 655360

void drawInformationText();
void draw(Model* model, mat4 mdlMatrix);
void drawLake();
void drawSkybox(mat4 worldToView);
void drawTerrain();
void DrawModelInstanced(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName, int count);
vec3 VectorReverse(vec3 a);


const float cameraHeight = 15.0;
vec3 camPos = { 10.0f, 20.0f, 0.0f };
vec3 camLookAt = { 0.0f, 0.0f, 0.0f };
vec3 camUp = { 0.0f, 1.0f, 0.0f };

GLuint isSnowing = 1;

GLfloat scaling_factor = 20.0;


void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	sfSetRasterSize(w, h);
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
GLuint bumpTex;


int simulationSpeed;
struct vec2int windDirection;

GLuint texprogram;

GLuint cubemap;

char *textureFileName[6] = 
{
	"./res/positive_z.tga",
  	"./res/negative_z.tga",
 	"./res/positive_y.tga",
  	"./res/negative_y.tga",
  	"./res/positive_x.tga",
  	"./res/negative_x.tga",
};

TextureData texData[6];

void loadTextures()
{
	glGenTextures(1, &cubemap);	
	glActiveTexture(GL_TEXTURE6);  
	
	for (int i = 0; i < 6; i++)
	{
		printf("Loading texture %s\n", textureFileName[i]);
		LoadTGATexture(textureFileName[i], &texData[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	printf("Texture loaded \n");
	// Load to cube map
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	printf("Texture binded\n");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, texData[0].w, texData[0].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData[0].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, texData[1].w, texData[1].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData[1].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, texData[2].w, texData[2].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData[2].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, texData[3].w, texData[3].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData[3].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, texData[4].w, texData[4].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData[4].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, texData[5].w, texData[5].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData[5].imageData);
	printf("Textures generated\n");
  
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	printf("Parameters sent \n");

// MIPMAPPING
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	printf("mipmapping done\n");
}

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
	skyModel = LoadModelPlus("./res/skyboxsnow.obj");
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


	simulationSpeed = 100;
	windDirection.x = 0;
	windDirection.z = 0;

	glUseProgram(snowprogram);
	glUniform1i(glGetUniformLocation(snowprogram, "heightTex"), 3);
	glUniform1i(glGetUniformLocation(snowprogram, "snowflakeTex"), 5);
	glUniform1i(glGetUniformLocation(snowprogram, "simulationSpeed"), simulationSpeed);
	glUniform1i(glGetUniformLocation(snowprogram, "isSnowing"), isSnowing);
	glUniform1i(glGetUniformLocation(snowprogram, "x_wind"), windDirection.x);
	glUniform1i(glGetUniformLocation(snowprogram, "z_wind"), windDirection.z);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "snowTex"), 4);
	glUniform1i(glGetUniformLocation(program, "cubemap"), 6);
	glUniform1i(glGetUniformLocation(program, "bumpTex"), 7);
	glUseProgram(texprogram);
	glUniform1i(glGetUniformLocation(texprogram, "tex"), 6);

	printError("GL init send texture unit numbers to shader");
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
	drawSkybox(worldToView);
	drawLake();
	drawTerrain();
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
	drawInformationText();
	printError("GL display draw text");


	glutSwapBuffers();
	printError("GL display swap buffers");
}

void timer(int i)
{
	glutTimerFunc(20, &timer, i);
	glutPostRedisplay();
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
	loadTextures();
	sfMakeRasterFont();
	sfSetRasterSize(WIN_X_SIZE, WIN_Y_SIZE);
	glutMainLoop();
	exit(0);
}

int previousTime = 0;
void drawInformationText()
{
	int t_current = glutGet(GLUT_ELAPSED_TIME);
	int fps = (int) (1000.0f/((float) (t_current - previousTime)));
	previousTime = t_current;

	char stringText[] = "Frames per second: ";
	char stringBuffer[128];
	snprintf(stringBuffer, sizeof(stringBuffer), "%s%d", stringText, fps);
	sfDrawString(20, WIN_Y_SIZE - 60, stringBuffer);
	char stringText2[] = "Simulation speed: ";
	snprintf(stringBuffer, sizeof(stringBuffer), "%s%i", stringText2, simulationSpeed);
	sfDrawString(20, WIN_Y_SIZE - 40, stringBuffer);
	char stringText3[] = "Number of snowflakes: ";
	snprintf(stringBuffer, sizeof(stringBuffer), "%s%i", stringText3, no_particles);
	sfDrawString(20, WIN_Y_SIZE - 20, stringBuffer);
	char stringText4[] = "Wind speed x-dir: ";
	snprintf(stringBuffer, sizeof(stringBuffer), "%s%i", stringText4, windDirection.x);
	sfDrawString(370, WIN_Y_SIZE - 40, stringBuffer);
	char stringText5[] = "Wind speed z-dir: ";
	snprintf(stringBuffer, sizeof(stringBuffer), "%s%i", stringText5, windDirection.z);
	sfDrawString(370, WIN_Y_SIZE - 20, stringBuffer);
}

void draw(Model* model, mat4 mdlMatrix)
{
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, mdlMatrix.m);
	DrawModel(model, program, "inPosition", "inNormal", "inTexCoord");
}

void drawLake()
{
	mat4 modelToWorld = T(0, -GetHeight(&lakeTexture, 0, 0), 0);

	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "drawing_lake_bottom"), 1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, modelToWorld.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	DrawModel(lakeModel, program, "inPosition", "inNormal", "inTexCoord");
	glUniform1i(glGetUniformLocation(program, "drawing_lake_bottom"), 0);
}

void drawSkybox(mat4 worldToView)
{
	glDisable(GL_DEPTH_TEST);
	glUseProgram(texprogram);
	worldToView.m[3] = 0;
	worldToView.m[7] = 0;
	worldToView.m[11] = 0;
	glUniformMatrix4fv(glGetUniformLocation(texprogram, "worldToViewMatrix"), 1, GL_TRUE, worldToView.m);

	for (int i = 0; i < 6; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texData[i].texID);
		DrawModel(box[i], texprogram, "inPosition", NULL, "inTexCoord");
	}

	glEnable(GL_DEPTH_TEST);
}

void drawTerrain()
{
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, IdentityMatrix().m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glUniform1i(glGetUniformLocation(program, "dirtTex"), 2);
	DrawModel(terrainModel, program, "inPosition", "inNormal", "inTexCoord"); // Draw terrain
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
