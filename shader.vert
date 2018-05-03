#version 450
#define no_particles 655360
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
out vec3 reflectedView;

uniform vec3 ftl;
uniform vec3 fbr;
uniform vec3 ntl;
uniform vec3 nbr;
uniform vec3 farNormal;
uniform vec3 leftNormal;
uniform vec3 rightNormal;
uniform vec3 topNormal;
uniform vec3 bottomNormal;
uniform vec3 cameraPos;
uniform int isSnowing;
uniform int drawing_lake_bottom;
uniform float time;
uniform sampler2D bumpTex;

 layout(std430, binding = 3) buffer layoutName
 {
    int snow[2*256*2*256];
    vec3 data_SSBO[no_particles];
 };

// NY
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;
uniform mat4 modelToWorldMatrix;

float directionalLight(vec3 lightDirection, 
                      float specularExponent, 
                      float useSpecular);
float snoise(vec2 v);

void main(void)
{
    vec3 lightDir = vec3(1,2,1);
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

  float shade = directionalLight(lightDir,0,0);

  // Make the snow melt on the mountains
  if(
    (snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)] > 0) 
    && (isSnowing == 0)
    && (shade > 0.6)
    && (snoise(vec2(time/10000, gl_VertexID)) > 0.5)
    && (exSurface.y > 0)
    )
  {
    snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)] -= int(10*shade);
  }

// Make the snow melt on the lake
  if(
    (snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)] > 0) 
    && (isSnowing == 0)
    && (snoise(vec2(time/10000, gl_VertexID)) > 0.5)
    && (exSurface.y <= 0)
    && (drawing_lake_bottom == 0)
    )
  {
    snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)] -= 8;
  }


	gl_Position = projMatrix * worldToViewMatrix* modelToWorldMatrix * vec4(inPosition, 1.0);

// modelToWorldMatrix = model to world
// worldToViewMatrix = world to view

// IMPORTANT! You MUST know what coordinate system you work in!
// The skybox is defined in world coordinates. Therefore we need to output a direction in
// world coordinates.
// First we transform the position to view coordinates, where it will be the same as the view direction
// (since the PRP is origin there).
  vec3 posInViewCoord = vec3(worldToViewMatrix * modelToWorldMatrix * vec4(inPosition, 1.0));
  vec3 viewDirectionInViewCoord = normalize(posInViewCoord);
// The we transform the resulting direction (now without translations) back to world coordinates
// using the inverse of the world-to-view.
  vec3 viewDirectionInWorldCoord = inverse(mat3(worldToViewMatrix)) * viewDirectionInViewCoord;

// Also transform the normal vector to world coordinates (= skybox coordinates).
    const float step = 1.0/256.0; // Should match size of texture
    float bt = texture(bumpTex, texCoord).r - texture(bumpTex, texCoord + vec2(0.0, step)).r;
    float bs = texture(bumpTex, texCoord).r - texture(bumpTex, texCoord + vec2(step, 0.0)).r;
    // fake it
    vec3 ps, pt;
    ps = 2.0 * cross(inNormal, vec3(1.0, 0.0, 0.0));
    pt = 2.0 * cross(inNormal, ps);
    vec3 n = inNormal + ps * bs + pt * bt;
  vec3 wcNormal = mat3(modelToWorldMatrix) * n;
// Using "reflect", we reflect the view direction to a reflected direction
  reflectedView = reflect(viewDirectionInWorldCoord, normalize(wcNormal)); // world coord = model of skybox


}

float directionalLight(vec3 lightDirection, float specularExponent, float useSpecular)
{
      // vertex shader sends all in world coordinates
    vec3 light = normalize(lightDirection);
    float diffuse, specular, shade;

    // Diffuse
    diffuse = dot(normalize(inNormal), light);
    diffuse = max(0.0, diffuse); // No negative light

    // Specular
    vec3 r = reflect(-light, normalize(inNormal));
    vec3 v = normalize(cameraPos - inPosition); // View direction
    specular = dot(r, v);
    if (specular > 0.0)
        specular = 1.0 * pow(specular, specularExponent);
    specular = max(specular, 0.0);
    shade = 0.7*diffuse + useSpecular*specular;
    return shade;
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