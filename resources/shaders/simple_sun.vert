#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout (std140) uniform CameraBlock{
	mat4 ViewMatrix;
 	mat4 ProjectionMatrix;
};

layout(location = 0) in vec3 in_Position;
layout(location = 2) in vec2 in_TexCoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;


out vec2 pass_TexCord;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	pass_TexCord = in_TexCoord;
}
