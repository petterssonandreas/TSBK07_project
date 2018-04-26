#version 450

out vec4 outColor;

in vec2 texCoord;
//in int InstanceID;
//in vec3 normal;
in vec3 exSurface;
in float discardFrag;
in float distanceToObject;

uniform sampler2D snowflakeTex;

void main(void)
{
	if (discardFrag > 0)
	{
		discard;
		return;
	}
    //outColor = texture(snowflakeTex, texCoord);
    //outColor = vec4(InstanceID,0,0,1);
    //outColor = vec4(data_SSBO[InstanceID],0,0,1);


    if (distanceToObject < 30.0)
    {
    	vec4 texColor = texture(snowflakeTex, texCoord);
    	if (texColor.w < 0.1)
    	{
    		discard;
    	}
    	outColor = texColor;
    }
    else
    {
    	outColor = vec4(1,1,1,1);
    }
    
    return;
}