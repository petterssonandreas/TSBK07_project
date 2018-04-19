#version 450
#define M_PI 3.1415926535897932384626433832795
#define no_particles 65536
#define scaling_factor 40.0
#define size_of_world 256.0
#define snowFactor 10.0

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord;

 layout(std430, binding = 3) buffer layoutName
 {
    int snow[no_particles*100];
    vec3 data_SSBO[no_particles];
 };

out vec2 texCoord;
out vec3 exSurface;

uniform sampler2D heightTex;
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;
uniform mat4 modelToWorldMatrix;
uniform float time;
uniform float time_0;
uniform int simulationSpeed;

float snoise(vec2 v);
 
void main(void)
{
	mat3 normalMatrix = mat3(modelToWorldMatrix);
	texCoord = inTexCoord;
  exSurface = vec3(modelToWorldMatrix * vec4(inPosition, 1.0)); // Send in world coordinates

  //Restart the snowflake in z-coord
  if (data_SSBO[gl_InstanceID].z == 0)
  {
  	float z_coord = 232.12;//size_of_world * snoise(vec2(gl_InstanceID,time/10000));
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
  if (data_SSBO[gl_InstanceID].x == 0)
  {
  	float x_coord = 75.123; //size_of_world * snoise(vec2(time/10000,gl_InstanceID));
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

  //Calculate the tex coord of the vertex to check the height
	float z_tex_coord = data_SSBO[gl_InstanceID].z/size_of_world;
	float x_tex_coord = data_SSBO[gl_InstanceID].x/size_of_world;

  //If the starting height has not been called, calculate
  if (data_SSBO[gl_InstanceID].y == 0)
  {
    data_SSBO[gl_InstanceID].y=  200 * snoise(vec2(gl_InstanceID*2,time/10000000));
  }

  data_SSBO[gl_InstanceID].y -= 0.0001 * simulationSpeed;
  float height = float(data_SSBO[gl_InstanceID].y);
	
  //Calculate the ground height at this vertex
	float ground_height = texture(heightTex, vec2(x_tex_coord, z_tex_coord)).x * size_of_world/ scaling_factor;

  //Prevent the snowflakes from falling through the ground
  if (height < ground_height)
  {
  	height = ground_height;

    //Use the value zero as flag to redistribute the snowflakes
    data_SSBO[gl_InstanceID].y += 50;
    data_SSBO[gl_InstanceID].x = 0;
    data_SSBO[gl_InstanceID].z = 0;

    //Accumulated sum of snow that has fallen here
    if (snow[int(2*x_coord)*int(size_of_world) + int(2*z_coord)] < snowFactor)
      snow[int(2*x_coord)*int(size_of_world) + int(2*z_coord)] += 1;
  }

	gl_Position = projMatrix * worldToViewMatrix * (modelToWorldMatrix * vec4(inPosition, 1.0) 
		+ vec4(x_coord, height, z_coord, 0));
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