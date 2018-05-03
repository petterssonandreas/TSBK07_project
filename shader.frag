#version 450
#define no_particles 655360
#define snowFactor 1000.0

out vec4 outColor;

in vec2 texCoord;
in vec3 normal;
in vec3 exSurface;
in float Color;
in float discardFrag;
in vec3 reflectedView;

uniform vec3 cameraPos;
uniform sampler2D tex;
uniform sampler2D dirtTex;
uniform sampler2D snowTex;
uniform sampler2D bumpTex;
uniform int drawing_lake_bottom;
uniform int drawing_skyBox;
uniform int drawing_objects;
uniform samplerCube cubemap;

float positionalLight(vec3 lightPos, 
                     float specularExponent, 
                     float useSpecular,
                     vec3 normalVec);
float directionalLight(vec3 lightDirection, 
                      float specularExponent, 
                      float useSpecular,
                      vec3 normalVec);

void main(void)
{
    vec3 lightDir = vec3(1,2,1);

    if(drawing_lake_bottom == 1 && exSurface.y == 0)
    {
        discard;
        return;
    }

    if(drawing_skyBox == 1)
    {
        outColor = texture(tex, texCoord);
        return;
    }

    if(drawing_objects == 1)
    {
        float shade = directionalLight(lightDir, 50.0, 0.5, normal);
        outColor = vec4(1.0 * shade, 0, 0, 1.0);
        return;
    }

    // The surface of the water bodies with reflection from the sky.
    if(exSurface.y == 0)
    {
    const float step = 1.0/256.0; // Should match size of texture
    float bt = texture(bumpTex, texCoord).r - texture(bumpTex, texCoord + vec2(0.0, step)).r;
    float bs = texture(bumpTex, texCoord).r - texture(bumpTex, texCoord + vec2(step, 0.0)).r;
    // fake it
    vec3 ps, pt;
    ps = 2.0 * cross(normal, vec3(1.0, 0.0, 0.0));
    pt = 2.0 * cross(normal, ps);
    vec3 n = normal + ps * bs + pt * bt;

    float shades = directionalLight(lightDir, 0.0, 0.0, n);
    float shade = directionalLight(lightDir, 0.0, 0.0, normal);
    vec4 snow = texture(snowTex, texCoord);

    // outColor = vec4(shade, shade, shade, 1.0);
    outColor = (1 - Color/snowFactor) * vec4(vec3(texture(cubemap, normalize(reflectedView)) * shades), 1.0) * vec4(0.2 + shade,0.2 + shade,0.7 + shade,0.8) + 
               (Color/snowFactor) * vec4(vec3(snow * 1/0.7*shade), 1.0);
    }
    else if(exSurface.y < 0)
    {
        outColor = texture(dirtTex, texCoord);
    }
    else
    {
        float shade = directionalLight(lightDir, 0.0, 0.0, normal);
        vec4 grass = texture(tex, texCoord);

        vec4 snow = texture(snowTex, texCoord);

        outColor = (1 - Color/snowFactor) * vec4(vec3(grass *1.5*shade) + 0.1, 1.0) + Color/snowFactor * vec4(vec3(snow * 1/0.7*shade), 1.0);
    }
}

float positionalLight(vec3 lightPos, float specularExponent, float useSpecular, vec3 normalVec)
{
    // lightPos in world coordinates
    vec3 lightDirection = normalize(lightPos - exSurface);

    return directionalLight(lightDirection, specularExponent, useSpecular, normalVec);
}

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
