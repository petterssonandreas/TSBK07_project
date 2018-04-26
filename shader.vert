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
out float discardFrag;

uniform vec3 ftl;
uniform vec3 fbr;
uniform vec3 ntl;
uniform vec3 nbr;
uniform vec3 farNormal;
uniform vec3 leftNormal;
uniform vec3 rightNormal;
uniform vec3 topNormal;
uniform vec3 bottomNormal;

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
    
 // Calculate frustum culling
  vec4 posView4 = worldToViewMatrix * vec4(exSurface.x, exSurface.y, exSurface.z, 1.0);
  vec3 posView = vec3(posView4.x, posView4.y, posView4.z);
  if (dot(posView, farNormal) < dot(ftl, farNormal) 
      || dot(posView, leftNormal) < dot(ftl, leftNormal) 
      || dot(posView, rightNormal) < dot(nbr, rightNormal)
      || dot(posView, topNormal) < dot(ntl, topNormal)
      || dot(posView, bottomNormal) < dot(nbr, bottomNormal))
  {
    // Particle outside frustum, discard fragment
    discardFrag = 1.0;
  }
  else
  {
    // Particle inside, draw fragment
    discardFrag = 0.0;
  }


	gl_Position = projMatrix * worldToViewMatrix* modelToWorldMatrix * vec4(inPosition, 1.0);
}
