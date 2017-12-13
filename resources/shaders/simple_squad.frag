#version 150

uniform sampler2D ColorTex;
in vec2 pass_TexCoord;

out vec4 out_Color;


void main() {

  out_Color = texture(ColorTex, pass_TexCoord);
}
