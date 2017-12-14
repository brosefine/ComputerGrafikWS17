#version 150

uniform sampler2D ColorTex;
uniform bool greyscale;
uniform bool blur;

in vec2 pass_TexCoord;
in vec4 gl_FragCoord;

out vec4 out_Color;


void main() {

	vec4 Color = texture(ColorTex, pass_TexCoord); 

	if(blur){
		vec2 pixelSize;
		pixelSize.x = pass_TexCoord.x / gl_FragCoord.x;
		pixelSize.y = pass_TexCoord.y / gl_FragCoord.y; 
		

		/*
			|20|21|22|		home is 11
			|10|11|12|		neighbouring pixels adressed in
			|00|01|02|		relative delta coordinates
		*/

		vec2 FragCoord_00 = vec2(gl_FragCoord.x - 1.0, 	gl_FragCoord.y - 1.0);
		vec2 FragCoord_01 = vec2(gl_FragCoord.x, 		gl_FragCoord.y - 1.0);
		vec2 FragCoord_02 = vec2(gl_FragCoord.x + 1.0, 	gl_FragCoord.y - 1.0);
		vec2 FragCoord_10 = vec2(gl_FragCoord.x - 1.0, 	gl_FragCoord.y);
		vec2 FragCoord_12 = vec2(gl_FragCoord.x + 1.0, 	gl_FragCoord.y);
		vec2 FragCoord_20 = vec2(gl_FragCoord.x - 1.0, 	gl_FragCoord.y + 1.0);
		vec2 FragCoord_21 = vec2(gl_FragCoord.x,	 	gl_FragCoord.y + 1.0);
		vec2 FragCoord_22 = vec2(gl_FragCoord.x + 1.0, 	gl_FragCoord.y + 1.0);

		vec2 TexCoord_00 = vec2(pixelSize.x * FragCoord_00.x, pixelSize.y * FragCoord_00.y);
		vec2 TexCoord_01 = vec2(pixelSize.x * FragCoord_01.x, pixelSize.y * FragCoord_01.y);
		vec2 TexCoord_02 = vec2(pixelSize.x * FragCoord_02.x, pixelSize.y * FragCoord_02.y);
		vec2 TexCoord_10 = vec2(pixelSize.x * FragCoord_10.x, pixelSize.y * FragCoord_10.y);
		vec2 TexCoord_12 = vec2(pixelSize.x * FragCoord_12.x, pixelSize.y * FragCoord_12.y);
		vec2 TexCoord_20 = vec2(pixelSize.x * FragCoord_20.x, pixelSize.y * FragCoord_20.y);
		vec2 TexCoord_21 = vec2(pixelSize.x * FragCoord_21.x, pixelSize.y * FragCoord_21.y);
		vec2 TexCoord_22 = vec2(pixelSize.x * FragCoord_22.x, pixelSize.y * FragCoord_22.y);

		Color = Color / 4.0;

		Color += texture(ColorTex, TexCoord_00) /16.0;
		Color += texture(ColorTex, TexCoord_01) /8.0;
		Color += texture(ColorTex, TexCoord_02) /16.0;
		Color += texture(ColorTex, TexCoord_10) /8.0;
		Color += texture(ColorTex, TexCoord_12) /8.0;
		Color += texture(ColorTex, TexCoord_20) /16.0;
		Color += texture(ColorTex, TexCoord_21) /8.0;
		Color += texture(ColorTex, TexCoord_22) /16.0;

		//Color = Color + Color_00 + Color_01 + Color_02 + Color_10 + Color_12 + Color_20 + Color_21 + Color_22;
	}


	if(greyscale){

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
