// shader.frag
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A fragment shader that handles all objects in the scene except for the sky-
// box and the snowflakes

#version 450
#define no_particles 327680
#define snowFactor 1000.0

// ----------- IN -------------
in vec2 texCoord;
in vec3 normal;
in vec3 exSurface;
in float Color;         // Color is the amount of snow accumulated on one spot
in vec3 reflectedView;

// ----------- OUT ------------
out vec4 outColor;

// ----------- UNIFORMS -------
uniform vec3 cameraPos;
uniform sampler2D tex;
uniform sampler2D dirtTex;
uniform sampler2D snowTex;
uniform sampler2D bumpTex;
uniform int drawing_lake_bottom;
uniform samplerCube cubemap;

float directionalLight(vec3 lightDirection, 
                      float specularExponent, 
                      float useSpecular,
                      vec3 normalVec);

void main(void)
{
    // One global light source
    vec3 lightDir = vec3(1,2,1);

    // Discard the objects of the lake bottom model that is not below the
    // surface
    if(drawing_lake_bottom == 1 && exSurface.y == 0)
    {
        discard;
        return;
    }

    // The surface of the water bodies with reflection from the sky.
    if(exSurface.y == 0)
    {
    const float step_length = 1.0/256.0; 
    float bt = texture(bumpTex, texCoord).r - 
               texture(bumpTex, texCoord + vec2(0.0, step_length)).r;
    float bs = texture(bumpTex, texCoord).r - 
               texture(bumpTex, texCoord + vec2(step_length, 0.0)).r;
    
    // Take the cross product with anything
    vec3 ps, pt;
    ps = 2.0 * cross(normal, vec3(1.0, 0.0, 0.0));
    pt = 2.0 * cross(normal, ps);
    vec3 n = normal + ps * bs + pt * bt;

    float shade_with_bump = directionalLight(lightDir, 0.0, 0.0, n);
    float shade = directionalLight(lightDir, 0.0, 0.0, normal);

    // The different "textures" that are blended on the lake
    vec4 snow = texture(snowTex, texCoord);
    vec4 reflection = texture(cubemap, normalize(reflectedView));
    vec4 transparency = vec4(0.2 + shade,0.2 + shade,0.7 + shade,0.8);

    // Blend the snow texture with the reflection of the skybox with bumped
    // normals
    outColor = (1 - Color/snowFactor) * 
                    vec4(vec3(reflection * shade_with_bump), 1.0) * 
                    transparency + 
               (Color/snowFactor) * 
               vec4(vec3(snow * 1/0.7*shade), 1.0);
    }

    // Give the bottom of the lake the dirt texture
    else if(exSurface.y < 0)
    {
        outColor = texture(dirtTex, texCoord);
    }

    // Give the terrain the correct texture
    else
    {
        float shade = directionalLight(lightDir, 0.0, 0.0, normal);

        // The textures to be blended
        vec4 grass = texture(tex, texCoord);
        vec4 snow = texture(snowTex, texCoord);

        outColor = (1 - Color/snowFactor) * vec4(vec3(grass *1.5*shade) + 0.1, 1.0) + Color/snowFactor * vec4(vec3(snow * 1/0.7*shade), 1.0);
    }
}

// Calculates the shading on objects
float directionalLight(vec3 lightDirection, float specularExponent, float useSpecular, vec3 normalVec)
{
    // vertex shader sends all in world coordinates
    vec3 light = normalize(lightDirection);
    float diffuse, specular, shade;

    // Diffuse
    diffuse = dot(normalize(normalVec), light);
    diffuse = max(0.0, diffuse); // No negative light

    // Specular
    vec3 r = reflect(-light, normalize(normalVec));
    vec3 v = normalize(cameraPos - exSurface); // View direction
    specular = dot(r, v);
    if (specular > 0.0)
        specular = 1.0 * pow(specular, specularExponent);
    specular = max(specular, 0.0);
    shade = 0.7*diffuse + useSpecular*specular;
    return shade;
}
