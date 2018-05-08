// snow_shader.frag
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// A fragment shader that handles the snowflakes and decides between snowflakes
// and white squares

#version 450

in vec2 texCoord;
in vec3 exSurface;
in float discardFrag;
in float distanceToObject;

out vec4 outColor;

uniform sampler2D snowflakeTex;

void main(void)
{
    // Frustum culling
	if (discardFrag > 0)
	{
		discard;
		return;
	}

    // LOD: if close: snowflake texture
    if (distanceToObject < 30.0)
    {
    	vec4 texColor = texture(snowflakeTex, texCoord);
    	if (texColor.w < 0.1)
    	{
    		discard;
    	}
    	outColor = texColor;
    }
    // LOD: if far away: white square
    else
    {
    	outColor = vec4(1,1,1,1);
    }
    
    return;
}