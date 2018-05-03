
#include "MicroGlut.h"
#include "VectorUtils3.h"
#include "LoadTGA.h"

#include "defines.h"
#include "initFunctions.h"

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



void loadSkyboxTextures(TextureData * texData)
{
	// Reference to cubemap
	GLuint cubemap;

	// Texture filenames for skybox
	char *textureFileName[6] = 
	{
		"./res/positive_z.tga",
	  	"./res/negative_z.tga",
	 	"./res/positive_y.tga",
	  	"./res/negative_y.tga",
	  	"./res/positive_x.tga",
	  	"./res/negative_x.tga",
	};


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