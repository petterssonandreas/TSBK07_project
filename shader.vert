#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 normal;
out vec3 exSurface;

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
	gl_Position = projMatrix * worldToViewMatrix* modelToWorldMatrix * vec4(inPosition, 1.0);
}
