#version 150

vec3 diffuse = vec3(0.5,0.5,0.5);
vec3 specular = vec3(0.5,0.5,0.5);
vec3 ambient = vec3(0.5,0.5,0.5);

in vec3 pass_Normal;
in vec3 pass_LightDir;
in vec3 pass_CameraDir;

in vec3 pass_matAmbient;
in vec3 pass_matDiffuse;
in vec3 pass_matSpecular;
in float pass_matShininess;

out vec4 out_Color;

vec3 ambientLighting(){

	return pass_matAmbient * ambient;
}

vec3 diffuseLighting(in vec3 N, in vec3 L){

	float diffuseTerm = clamp(dot(N,L),0,1);
	return pass_matDiffuse * diffuse * diffuseTerm;

}

vec3 specularLighting(in vec3 N, in vec3 L, in vec3 V){
	
	float specularTerm = 0;

	if(dot(N, L) > 0){
		vec3 H = normalize(L+V);
		specularTerm = pow(dot(N,H), pass_matShininess);
	}

	return pass_matSpecular * specular * specularTerm;
}

void main() {

	float outline = 0.35;
	float shadow =	0.5;
	float medium =  0.6;
	float light = 0.7;
	
	vec3 L = normalize(pass_LightDir);
	vec3 V = normalize(pass_CameraDir);
	vec3 N = normalize(pass_Normal);

	float dotView = dot(pass_Normal, pass_CameraDir);

	if(dotView < outline){
		out_Color = vec4(pass_matAmbient, 1.0);
	} else if (dotView < shadow) {
		out_Color = vec4(1.0, 0.0, 0.0, 1.0);
	} else if (dotView < medium) {
		out_Color = vec4(0.0, 1.0, 0.0, 1.0);
	} else {
		out_Color = vec4(0.0, 0.0, 1.0, 1.0);
	}

	//{
	//
	//	vec3 amb = ambientLighting();
	//	vec3 dif = diffuseLighting(N, L);
	//	vec3 spe = specularLighting(N, L, V);
	//
	//
	//	out_Color = vec4((amb + dif + spe), 1.0);
	//}

}
