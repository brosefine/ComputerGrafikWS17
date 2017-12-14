#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;

uniform int	mirrored_v;
uniform int	mirrored_h;

out vec2 pass_TexCoord;

void main(void)
{
	vec2 TexCoord = in_TexCoord;

	if(mirrored_v == 1){
		if(in_TexCoord.y == 1) 	{TexCoord.y = 0.0;}
		else 					{TexCoord.y = 1.0;}
	}

	gl_Position = vec4(in_Position, 1.0);
	pass_TexCoord = in_TexCoord;
}
