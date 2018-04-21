#version 450
#define no_particles 65536
#define size_of_world 256
//#define imageScale 4

uniform int imageScale;

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 normal;
out vec3 exSurface;
out float Color;

 layout(std430, binding = 3) buffer layoutName
 {
    int snow[8*256*8*256];
    vec3 data_SSBO[no_particles];
 };

// NY
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;
uniform mat4 modelToWorldMatrix;

void main(void)
{
	mat3 normalMatrix = mat3(modelToWorldMatrix);
    normal = normalMatrix * inNormal;
	texCoord = inTexCoord;
    exSurface = vec3(modelToWorldMatrix * vec4(inPosition, 1.0)); // Send in world coordinates
    Color = snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)];// == 1)
    //{
    //	Color = 1.0;
    //}
    //else
    //{
     //   Color = 0.0;
    //}
	gl_Position = projMatrix * worldToViewMatrix* modelToWorldMatrix * vec4(inPosition, 1.0);
}
