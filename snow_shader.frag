#version 450

out vec4 outColor;

in vec2 texCoord;
//in int InstanceID;
//in vec3 normal;
in vec3 exSurface;

 layout(std430, binding = 3) buffer layoutName
 {
     int data_SSBO[];
 };


uniform sampler2D tex;

void main(void)
{
    //outColor = texture(tex, texCoord);
    //outColor = vec4(InstanceID,0,0,1);
    //outColor = vec4(data_SSBO[InstanceID],0,0,1);
    outColor = vec4(1,1,1,1);
    return;
}