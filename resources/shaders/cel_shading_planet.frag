#version 150

vec3 diffuse = vec3(0.5,0.5,0.5);
vec3 specular = vec3(0.5,0.5,0.5);
vec3 ambient = vec3(0.5,0.5,0.5);

in vec3 pass_Normal;
in vec3 pass_LightDir;
in vec3 pass_CameraDir;

in vec3 pass_matAmbient;
//in vec3 pass_matDiffuse;
//in vec3 pass_matSpecular;
//in float pass_matShininess;

out vec4 out_Color;

float diffuseLighting(in vec3 N, in vec3 L){

	return clamp(dot(L,N),0.0,1.0);

}

float specularLighting(in vec3 N, in vec3 L, in vec3 V){
	
	if(dot(N,L)>0){
		vec3 H = normalize(L+V);
      return pow(clamp(dot(H,N),0.0,1.0),64.0);
   	}
   return 0.0;
}

void main() {

	float outline = 0.35;
	float shades = 3;
	
	vec3 L = normalize(pass_LightDir);
	vec3 V = normalize(pass_CameraDir);
	vec3 N = normalize(pass_Normal);

	float dotView = dot(pass_Normal, pass_CameraDir);

	if(dotView < outline){
		out_Color = vec4(pass_matAmbient, 1.0);
	} else {
	
		float amb = 0.1;
		float dif = diffuseLighting(N, L);
		float spe = specularLighting(N, L, V);

		float intensity = amb + dif + spe;
		float shadeIntensity = ceil(intensity * shades)/shades;
	
	
		out_Color = vec4((pass_matAmbient * shadeIntensity), 1.0);
	}

}
