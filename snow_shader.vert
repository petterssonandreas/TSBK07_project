// snow_shader.vert
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A vertex shader that handles the snowflakes and their physics. 

#version 450
#define no_particles 327680
#define no_vertices 262144
#define scaling_factor 20.0 // For the hight-map. Same as in main.c
#define size_of_world 256
#define snowFactor 1000.0

// ----------- IN -------------
in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord;

// ----------- OUT ------------
out vec2 texCoord;
out vec3 exSurface;
out float discardFrag;
out float distanceToObject;

// ----------- UNIFORMS -------
uniform sampler2D heightTex;
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 scaleMatrix;
uniform float time;
uniform int simulationSpeed;
uniform int imageScale;
uniform int isSnowing;
uniform vec3 cameraPos;
// For frustum culling
uniform vec3 ftl;
uniform vec3 fbr;
uniform vec3 ntl;
uniform vec3 nbr;
uniform vec3 farNormal;
uniform vec3 leftNormal;
uniform vec3 rightNormal;
uniform vec3 topNormal;
uniform vec3 bottomNormal;
// For wind
uniform int x_wind;
uniform int z_wind;

layout(std430, binding = 0) buffer layoutName
{
  int snow[no_vertices];
  vec3 data_SSBO[no_particles];
};

// The random function, v is the 2D-seed
float snoise(vec2 v);
 
void main(void)
{
	mat3 normalMatrix = mat3(modelToWorldMatrix);
	texCoord = inTexCoord;
  // Send in world coordinates
  exSurface = vec3(modelToWorldMatrix * vec4(inPosition, 1.0)); 

  //Restart the snowflake in z-coord
  if ((data_SSBO[gl_InstanceID].z == 0) && (isSnowing == 1))
  {
  	float z_coord = size_of_world * snoise(vec2(gl_InstanceID,time/100000));
  	while (z_coord > size_of_world)
  	{
  		z_coord = z_coord - size_of_world;
  	}
  	while (z_coord < 0.0)
  	{
  		z_coord = z_coord + size_of_world;
  	}
    data_SSBO[gl_InstanceID].z = z_coord;
  }
  float z_coord = data_SSBO[gl_InstanceID].z;

  //Restart the snowflake in x-coord
  if ((data_SSBO[gl_InstanceID].x == 0) && (isSnowing == 1))
  {
  	float x_coord = size_of_world * snoise(vec2(time/100000,gl_InstanceID));
  	while (x_coord > size_of_world)
  	{
  		x_coord = x_coord - size_of_world;
  	}
  	while (x_coord < 0.0)
  	{
  		x_coord = x_coord + size_of_world;
  	}
    data_SSBO[gl_InstanceID].x = x_coord;
  }
  float x_coord = data_SSBO[gl_InstanceID].x;

  // Update position due to wind
  if (x_wind != 0)
  {
    x_coord += float(x_wind) * 0.00001 * simulationSpeed;
    while (x_coord > size_of_world)
    {
    	x_coord = x_coord - size_of_world;
    }
    while (x_coord < 0.0)
    {
    	x_coord = x_coord + size_of_world;
    }
    data_SSBO[gl_InstanceID].x = x_coord;
  }

  if (z_wind != 0)
  {
    z_coord += float(z_wind) * 0.00001 * simulationSpeed;
    while (z_coord > size_of_world)
    {
    	z_coord = z_coord - size_of_world;
    }
    while (z_coord < 0.0)
    {
    	z_coord = z_coord + size_of_world;
    }
    data_SSBO[gl_InstanceID].z = z_coord;
  }

  //Calculate the tex coord of the vertex to check the height
	float z_tex_coord = z_coord/size_of_world;
	float x_tex_coord = x_coord/size_of_world;

  //If the starting height has not been called, calculate
  if ((data_SSBO[gl_InstanceID].y == 0) && (isSnowing == 1))
  {
    float height = 100 * snoise(vec2(gl_InstanceID*2,time/10000000));
    height += 150;
    data_SSBO[gl_InstanceID].y = height;
  }

  data_SSBO[gl_InstanceID].y -= 0.0001 * simulationSpeed;
  float height = data_SSBO[gl_InstanceID].y;
	
  //Calculate the ground height at this vertex
	float ground_height = texture(heightTex, vec2(x_tex_coord, z_tex_coord)).x * size_of_world/ scaling_factor;

  //Prevent the snowflakes from falling through the ground
  if (height < ground_height)
  {
  	height = ground_height;

    //Use the value zero as flag to redistribute the snowflakes
    data_SSBO[gl_InstanceID].y = 0;
    data_SSBO[gl_InstanceID].x = 0;
    data_SSBO[gl_InstanceID].z = 0;

    //Accumulated sum of snow that has fallen here
    if (snow[int(imageScale*x_coord)*imageScale*size_of_world + 
             int(imageScale*z_coord)] < snowFactor)
        snow[int(imageScale*x_coord)*imageScale*size_of_world + 
             int(imageScale*z_coord)] += 100;
  }

  // Calculate frustum culling
  vec4 posView4 = worldToViewMatrix * vec4(x_coord, height, z_coord, 1.0);
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

  // Rotations to get the billboards to face the camera
  vec3 particlePos = vec3(x_coord, height, z_coord);
  distanceToObject = length(cameraPos - particlePos);

  // Create matrix for translation
  mat4 translationMatrix = mat4(1.0);
  translationMatrix[3] = vec4(particlePos, 1.0);
  mat4 modelToView = worldToViewMatrix * translationMatrix;

  // Remove rotations to get billboard
  modelToView[0][0] = 1.0;
  modelToView[1][0] = 0.0;
  modelToView[2][0] = 0.0;
  modelToView[0][1] = 0.0;
  modelToView[1][1] = 1.0;
  modelToView[2][1] = 0.0;
  modelToView[0][2] = 0.0;
  modelToView[1][2] = 0.0;
  modelToView[2][2] = 1.0;
  
  gl_Position = projMatrix * modelToView * scaleMatrix * vec4(inPosition, 1.0);
}


//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
{
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}