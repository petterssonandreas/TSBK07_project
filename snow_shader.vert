#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;

out vec2 texCoord;
//out vec3 normal;
out vec3 exSurface;

uniform sampler2D heightTex;
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;
uniform mat4 modelToWorldMatrix;
uniform float time;
uniform float time_0;

float rand(vec2 co);

void main(void)
{
	mat3 normalMatrix = mat3(modelToWorldMatrix);
    //normal = normalMatrix * inNormal;
	texCoord = inTexCoord;
    exSurface = vec3(modelToWorldMatrix * vec4(inPosition, 1.0)); // Send in world coordinates

	float z_coord = float(gl_InstanceID % 256);
	float x_coord = float(gl_InstanceID / 256);
	float z_tex_coord = z_coord/256.0;
	float x_tex_coord = x_coord/256.0;

	float z_coord_noised = z_coord + 1 * 0.1 * rand(vec2(gl_InstanceID,time / 10000));
	float x_coord_noised = x_coord + 1 * 0.1 * rand(vec2(time / 10000,gl_InstanceID));

    float height = 20 - 0.001*(time-time_0);
	float ground_height = texture(heightTex, vec2(x_tex_coord, z_tex_coord)).x * 256.0/ 20.0;

   	if (height < ground_height)
   	{
   		height = ground_height;
		x_coord_noised = x_coord;
		z_coord_noised = z_coord;
   	}
	

	gl_Position = projMatrix * worldToViewMatrix * (modelToWorldMatrix * vec4(inPosition, 1.0) 
		+ vec4(x_coord_noised, height, z_coord_noised, 0));
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
