#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 normal;
out vec3 exSurface;

uniform sampler2D heightTex;

// NY
uniform mat4 projMatrix;
uniform mat4 worldToViewMatrix;
uniform mat4 modelToWorldMatrix;
uniform float time;
uniform float time_0;
uniform float heights[100];
void main(void)
{
	mat3 normalMatrix = mat3(modelToWorldMatrix);
    normal = normalMatrix * inNormal;
	texCoord = inTexCoord;
    exSurface = vec3(modelToWorldMatrix * vec4(inPosition, 1.0)); // Send in world coordinates
    float inst = float(gl_InstanceID);
	float z_coord = inst/256.0;
    int pos = int(inst * 256.0);
    float height = 10 - 0.001*(time-time_0);
   	if (height < 0)
   	{
   		height = 0;
   	}
	gl_Position = projMatrix * worldToViewMatrix * modelToWorldMatrix * (vec4(inPosition, 1.0) 
		+ vec4(0, texture(heightTex, vec2(0, z_coord)).x * 256.0/ 20.0, gl_InstanceID, 0));
}
