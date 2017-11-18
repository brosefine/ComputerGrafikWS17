#version 150

vec3 diffuse{0.5f,0.5f,0.5f};
vec3 specular{0.5f,0.5f,0.5f};
vec3 ambient{0.5f,0.5f,0.5f};

in vec3 pass_Normal;
in vec4 pass_LightDir;
in vec3 pass_CameraDir;

in vec3 matAmbient;
in vec3 matDiffuse;
in vec3 matSpecular;
in float matShininess;

out vec4 out_Color;

void main() {
  out_Color = pass_Color;
}
