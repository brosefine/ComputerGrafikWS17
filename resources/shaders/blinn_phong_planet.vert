#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matShininess;

out vec3 pass_Normal;
out vec3 pass_LightDir;
out vec3 pass_CameraDir;

out vec3 pass_matAmbient;
out vec3 pass_matDiffuse;
out vec3 pass_matSpecular;
out float pass_matShininess;

void main(void)
{
	vec4 sunPosition = ViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
	vec4 worldPosition = (ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);

	pass_Normal = normalize((NormalMatrix * vec4(in_Normal, 0.0)).xyz);
	pass_LightDir = normalize(sunPosition.xyz - worldPosition.xyz);
	pass_CameraDir = normalize(-1*(worldPosition.xyz));

	pass_matAmbient = matAmbient;
	pass_matDiffuse = matDiffuse;
	pass_matSpecular = matSpecular;
	pass_matShininess = matShininess;

	gl_Position = ProjectionMatrix * worldPosition;
}
