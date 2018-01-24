#version 150

/// Our light scattering pass texture
uniform sampler2D ColorTex;
/// Indicate where is the light source on the screen (2D position)
uniform vec2 lightPosition;

in vec2 pass_TexCoord;
out vec4 out_Color;

void main()
{
 float decay=0.96815;
 float exposure=0.2;
 float density=0.926;
 float weight=0.58767;

 /// NUM_SAMPLES will describe the rays quality, you can play with
 int NUM_SAMPLES = 100;

 vec2 tc = pass_TexCoord;
 vec2 deltaTexCoord = vec2(tc — lightPosition);
 deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * density;
 float illuminationDecay = 1.0;

 vec4 color = texture2D(ColorTex, tc.xy)*0.4;

 for(int i=0; i < NUM_SAMPLES ; i++)
 {
    tc -= deltaTexCoord;
    vec4 sample = texture2D(ColorTex, tc)*0.4;
    sample *= illuminationDecay * weight;
    color += sample;
    illuminationDecay *= decay;
 }

 out_Color = color;

}