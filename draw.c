

#include "MicroGlut.h"
#include "loadobj.h"
#include "VectorUtils3.h"

#include "defines.h"
#include "draw.h"
#include "simplefont.h"
#include "skybox.h"
#include "heightMap.h"

GLuint program;
GLuint snowprogram;
GLuint texprogram;

void setShaderPrograms(GLuint program_, GLuint snowprogram_, GLuint texprogram_)
{
	program = program_;
	snowprogram = snowprogram_;
	texprogram = texprogram_;
}


int previousTime = 0;
void drawInformationText(int simulationSpeed, struct vec2int windDirection)
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

void drawLake(Model* lakeModel, TextureData lakeTexture)
{
	mat4 modelToWorld = T(0, -GetHeight(&lakeTexture, 0, 0), 0);

	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "drawing_lake_bottom"), 1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelToWorldMatrix"), 1, GL_TRUE, modelToWorld.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	DrawModel(lakeModel, program, "inPosition", "inNormal", "inTexCoord");
	glUniform1i(glGetUniformLocation(program, "drawing_lake_bottom"), 0);
}

void drawSkybox(mat4 worldToView, TextureData * texData)
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

void drawTerrain(Model * terrainModel)
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
