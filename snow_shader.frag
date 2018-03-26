#version 150

out vec4 outColor;

in vec2 texCoord;
//in vec3 normal;
in vec3 exSurface;

uniform sampler2D tex;

void main(void)
{
    outColor = texture(tex, texCoord);
    return;
}