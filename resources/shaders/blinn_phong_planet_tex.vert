#version 150
#extension GL_ARB_explicit_attrib_location : require

layout (std140) uniform CameraBlock{
	mat4 ViewMatrix;
 	mat4 ProjectionMatrix;
};

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 NormalMatrix;


//pass to frag-shader
out vec2 pass_TexCord;

out vec3 pass_Normal;
out vec3 pass_LightDir;
out vec3 pass_CameraDir;

void main(void)
{
	//Sun at (0, 0, 0), translated to viewspace
	vec4 sunPosition = ViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
	//translate vertex position to viewspace
	vec4 worldPosition = (ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);

	//calculate direction vectors for shading
	pass_Normal = normalize((NormalMatrix * vec4(in_Normal, 0.0)).xyz);
	pass_LightDir = normalize(sunPosition.xyz - worldPosition.xyz);
	pass_CameraDir = normalize(-1*(worldPosition.xyz));

	pass_TexCord = in_TexCoord;

	gl_Position = ProjectionMatrix * worldPosition;
}
