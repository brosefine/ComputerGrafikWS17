#version 150

//light values
vec3 diffuse = vec3(0.5,0.5,0.5);
vec3 specular = vec3(0.5,0.5,0.5);
vec3 ambient = vec3(0.5,0.5,0.5);
//material color values
vec3 pass_matSpecular = vec3(1.0, 1.0, 1.0);
float pass_matShininess = 50.0;


in vec3 pass_Normal;
in vec3 pass_LightDir;
in vec3 pass_CameraDir;

//material color values
in vec3 pass_matAmbient;
in vec3 pass_matDiffuse;

out vec4 out_Color;

//calculate ambient color part (rgb)
vec3 ambientLighting(){

	return pass_matAmbient * ambient;
}

//calculate diffuse color part (rgb)
vec3 diffuseLighting(in vec3 N, in vec3 L){

	float diffuseTerm = clamp(dot(N,L),0,1);
	return pass_matDiffuse * diffuse * diffuseTerm;

}

//calculate specular color part (rgb)
vec3 specularLighting(in vec3 N, in vec3 L, in vec3 V){
	
	float specularTerm = 0;

	if(dot(N, L) > 0){
		vec3 H = normalize(L+V);
		specularTerm = pow(dot(N,H), pass_matShininess);
	}

	return pass_matSpecular * specular * specularTerm;
}

void main() {
	
	vec3 L = normalize(pass_LightDir);
	vec3 V = normalize(pass_CameraDir);
	vec3 N = normalize(pass_Normal);

	vec3 amb = ambientLighting();
	vec3 dif = diffuseLighting(N, L);
	vec3 spe = specularLighting(N, L, V);

	//output color combines all parts (ambient, diffuse, specular)
	out_Color = vec4((amb + dif + spe), 1.0);
}
