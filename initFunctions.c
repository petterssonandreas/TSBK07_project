
#include "MicroGlut.h"
//#include "loadobj.h"
#include "VectorUtils3.h"

#include "defines.h"

#define NO_VERTICES (2*256*2*256)


void createSSBO()
{
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}