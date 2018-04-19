#version 450
#define no_particles 65536

out vec4 outColor;

in vec2 texCoord;
in vec3 normal;
in vec3 exSurface;
in float Color;

uniform vec3 cameraPos;
uniform sampler2D tex;
uniform sampler2D dirtTex;
uniform int drawing_lake_bottom;
uniform int drawing_skyBox;
uniform int drawing_objects;

float positionalLight(vec3 lightPos, float specularExponent, float useSpecular);
float directionalLight(vec3 lightDirection, float specularExponent, float useSpecular);


void main(void)
{
    vec3 lightDir = vec3(-1,4,-1);

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
        float shade = directionalLight(lightDir, 50.0, 0.5);
        outColor = vec4(1.0 * shade, 0, 0, 1.0);
        return;
    }

    if(exSurface.y == 0)
    {
        float shade = directionalLight(lightDir, 1000.0, 1.0);
        outColor = vec4(0.2 + shade, 0.2 + shade, 0.7 + shade, 0.5);
    }
    else if(exSurface.y < 0)
    {
        outColor = texture(dirtTex, texCoord);
    }
    else
    {
        float shade = directionalLight(lightDir, 0.0, 0.0);
        vec4 grassdirt = (((1 - abs(normal.x)) * texture(tex, texCoord) + 
                            abs(normal.x) * texture(dirtTex, texCoord)) +  
                         ((1 - abs(normal.z)) * texture(tex, texCoord) + 
                            abs(normal.z) * texture(dirtTex, texCoord))); 

        float snowFactor = 10.0;

        outColor = (1 - Color/snowFactor) * grassdirt * vec4(vec3(2 * shade), 1.0)  * 0.5 + 
                        (Color/snowFactor) * vec4(vec3(2 * shade),1.0);
                 
        // outColor = vec4(Color,Color,Color,1);
        // if (0)//snow[int(exSurface.x*256 + exSurface.z)] == 1)
        // {
        //     outColor = vec4(1,1,1,1);
        // }
        // else
        // { 
        //     float shade = directionalLight(lightDir, 0.0, 0.0);
        //     outColor = vec4(vec3(2 * shade), 1.0)  * 0.5 *
        //         (((1 - abs(normal.x)) * texture(tex, texCoord) + abs(normal.x) * texture(dirtTex, texCoord)) +
        //         ((1 - abs(normal.z)) * texture(tex, texCoord) + abs(normal.z) * texture(dirtTex, texCoord)));
        // }
    }
}

float positionalLight(vec3 lightPos, float specularExponent, float useSpecular)
{
    // lightPos in world coordinates
    vec3 lightDirection = normalize(lightPos - exSurface);

    return directionalLight(lightDirection, specularExponent, useSpecular);
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
