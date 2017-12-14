#version 150

uniform sampler2D ColorTex;
uniform int				greyscale;
uniform int				blur;

in vec2 pass_TexCoord;

out vec4 out_Color;


void main() {

	vec4 Color = texture(ColorTex, pass_TexCoord); 

	

	if(greyscale == 1){

		float r = Color.x * 0.2126;
		float g = Color.y * 0.7152;
		float b = Color.z * 0.0722;

		float lum = r + g + b;

		Color.x = lum;
		Color.y = lum;
		Color.z = lum;

	}

  out_Color = Color;
}
