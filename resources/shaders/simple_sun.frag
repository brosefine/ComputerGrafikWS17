#version 150

uniform sampler2D ColorTex;
in vec2 pass_TexCord;
in vec3 pass_Normal;

out vec4 out_Color;


void main() {
//sun Color
  out_Color = texture(ColorTex, pass_TexCord);
}
