#version 150
#extension GL_ARB_explicit_attrib_location : require

layout (std140) uniform CameraBlock{
	mat4 ViewMatrix;
 	mat4 ProjectionMatrix;
};

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
}
