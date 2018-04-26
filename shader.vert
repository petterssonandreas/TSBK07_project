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
uniform vec3 cameraPos;
uniform int isSnowing;
uniform int meltingFactor;
uniform float time;

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
vec2 cellular(vec2 P);

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
  if(
    (snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)] > 0) 
    && (isSnowing == 0)
    && (shade > 0.6)
    && (cellular(vec2(time/1000, gl_VertexID)).y > 0.95)
    )
  {
    snow[int(imageScale*exSurface.x)*imageScale*size_of_world 
               + int(imageScale*exSurface.z)] -= int(10*shade);
  }


	gl_Position = projMatrix * worldToViewMatrix* modelToWorldMatrix * vec4(inPosition, 1.0);
}

float directionalLight(vec3 lightDirection, float specularExponent, float useSpecular)
{
    // vertex shader sends all in world coordinates
    vec3 light = normalize(lightDirection);
    float diffuse, specular, shade;

    // Diffuse
    diffuse = dot(normalize(normal), light);
    diffuse = max(0.0, diffuse); // No negative light

    // Specular
    vec3 r = reflect(-light, normalize(normal));
    vec3 v = normalize(cameraPos - exSurface); // View direction
    specular = dot(r, v);
    if (specular > 0.0)
        specular = 1.0 * pow(specular, specularExponent);
    specular = max(specular, 0.0);
    shade = 0.7*diffuse + useSpecular*specular;
    return shade;
}

// Cellular noise ("Worley noise") in 2D in GLSL.
// Copyright (c) Stefan Gustavson 2011-04-19. All rights reserved.
// This code is released under the conditions of the MIT license.
// See LICENSE file for details.
// https://github.com/stegu/webgl-noise

// Modulo 289 without a division (only multiplications)
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

// Modulo 7 without a division
vec3 mod7(vec3 x) {
  return x - floor(x * (1.0 / 7.0)) * 7.0;
}

// Permutation polynomial: (34x^2 + x) mod 289
vec3 permute(vec3 x) {
  return mod289((34.0 * x + 1.0) * x);
}

// Cellular noise, returning F1 and F2 in a vec2.
// Standard 3x3 search window for good F1 and F2 values
vec2 cellular(vec2 P) 
{
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 3/7
#define jitter 1.0 // Less gives more regular pattern
  vec2 Pi = mod289(floor(P));
  vec2 Pf = fract(P);
  vec3 oi = vec3(-1.0, 0.0, 1.0);
  vec3 of = vec3(-0.5, 0.5, 1.5);
  vec3 px = permute(Pi.x + oi);
  vec3 p = permute(px.x + Pi.y + oi); // p11, p12, p13
  vec3 ox = fract(p*K) - Ko;
  vec3 oy = mod7(floor(p*K))*K - Ko;
  vec3 dx = Pf.x + 0.5 + jitter*ox;
  vec3 dy = Pf.y - of + jitter*oy;
  vec3 d1 = dx * dx + dy * dy; // d11, d12 and d13, squared
  p = permute(px.y + Pi.y + oi); // p21, p22, p23
  ox = fract(p*K) - Ko;
  oy = mod7(floor(p*K))*K - Ko;
  dx = Pf.x - 0.5 + jitter*ox;
  dy = Pf.y - of + jitter*oy;
  vec3 d2 = dx * dx + dy * dy; // d21, d22 and d23, squared
  p = permute(px.z + Pi.y + oi); // p31, p32, p33
  ox = fract(p*K) - Ko;
  oy = mod7(floor(p*K))*K - Ko;
  dx = Pf.x - 1.5 + jitter*ox;
  dy = Pf.y - of + jitter*oy;
  vec3 d3 = dx * dx + dy * dy; // d31, d32 and d33, squared
  // Sort out the two smallest distances (F1, F2)
  vec3 d1a = min(d1, d2);
  d2 = max(d1, d2); // Swap to keep candidates for F2
  d2 = min(d2, d3); // neither F1 nor F2 are now in d3
  d1 = min(d1a, d2); // F1 is now in d1
  d2 = max(d1a, d2); // Swap to keep candidates for F2
  d1.xy = (d1.x < d1.y) ? d1.xy : d1.yx; // Swap if smaller
  d1.xz = (d1.x < d1.z) ? d1.xz : d1.zx; // F1 is in d1.x
  d1.yz = min(d1.yz, d2.yz); // F2 is now not in d2.yz
  d1.y = min(d1.y, d1.z); // nor in  d1.z
  d1.y = min(d1.y, d2.x); // F2 is in d1.y, we're done.
  return sqrt(d1.xy);
}