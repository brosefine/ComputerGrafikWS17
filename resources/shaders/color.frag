#version 150

vec3 diffuse{0.5f,0.5f,0.5f};
vec3 specular{0.5f,0.5f,0.5f};
vec3 ambient{0.5f,0.5f,0.5f};

in  vec3 pass_Normal;
in  vec4 pass_Color;
in  vec3 pass_sunPosition;

out vec4 out_Color;

void main() {
  out_Color = pass_Color;
}
