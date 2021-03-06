#version 150

uniform sampler2D ColorTex;
uniform sampler2D LightTex;
uniform vec4 lightPosition;
uniform bool greyscale;
uniform bool blur;
uniform bool scatter;

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

		vec4 Color_00 = texture(ColorTex, TexCoord_00) /16.0;
		vec4 Color_01 = texture(ColorTex, TexCoord_01) /8.0;
		vec4 Color_02 = texture(ColorTex, TexCoord_02) /16.0;
		vec4 Color_10 = texture(ColorTex, TexCoord_10) /8.0;
		vec4 Color_12 = texture(ColorTex, TexCoord_12) /8.0;
		vec4 Color_20 = texture(ColorTex, TexCoord_20) /16.0;
		vec4 Color_21 = texture(ColorTex, TexCoord_21) /8.0;
		vec4 Color_22 = texture(ColorTex, TexCoord_22) /16.0;

		Color = Color + Color_00 + Color_01 + Color_02 + Color_10 + Color_12 + Color_20 + Color_21 + Color_22;
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

	if(!scatter){
		/// NUM_SAMPLES will describe the rays quality, you can play with
		int NUM_SAMPLES = 250;

		float decay=0.9;
		float exposure=0.2;
		float density=5.0;
		float weight=0.55;
		//lightPos in texture coordinates
		vec2 lightPos = (lightPosition.xy + vec2(1.0, 1.0))/2;
		vec2 tc = pass_TexCoord;
		vec2 deltaTexCoord = (tc - lightPos) / float(NUM_SAMPLES) * density;
		float illuminationDecay = 1.0;

		vec4 light = texture2D(LightTex, tc);

		for(int i=0; i < NUM_SAMPLES ; i++){
			tc -= deltaTexCoord;
			vec4 sample = texture2D(LightTex, tc);
			sample *= illuminationDecay * weight;
			light += sample;
			illuminationDecay *= decay;
		}

		Color += light * exposure;

	}

  out_Color = Color;
}
