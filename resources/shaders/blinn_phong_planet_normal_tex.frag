#version 150

uniform sampler2D ColorTex;
uniform sampler2D NormalTex;

//light values
vec3 diffuse = vec3(0.5,0.5,0.5);
vec3 specular = vec3(0.5,0.5,0.5);
vec3 ambient = vec3(0.5,0.5,0.5);
//material color values
vec3 pass_matSpecular = vec3(1.0, 1.0, 1.0);
float pass_matShininess = 10.0;

in vec3 pass_Normal;
in vec3 pass_Tangent;
in vec3 pass_LightDir;
in vec3 pass_CameraDir;

//texture color
in vec2 pass_TexCord;

out vec4 out_Color;

vec3 Color = texture(ColorTex, pass_TexCord).xyz;
vec3 Normal = (vec3((texture(NormalTex, pass_TexCord).xy * 2.0 - 1.0), texture(NormalTex, pass_TexCord).z));
//vec3 Normal = vec3(texture(NormalTex, pass_TexCord).xyz * 2.0 - 1.0);
//calculate ambient color part (rgb)
vec3 ambientLighting(){

	return Color * ambient;
}

//calculate diffuse color part (rgb)
vec3 diffuseLighting(in vec3 N, in vec3 L){

	float diffuseTerm = max(dot(N,L),0);
	return Color * diffuse * diffuseTerm;

}

//calculate specular color part (rgb)
vec3 specularLighting(in vec3 N, in vec3 L, in vec3 V){
	
	float specularTerm = 0;

	if(dot(N, L) > 0){
		vec3 H = normalize(L+V);
		specularTerm = pow(max(dot(N,H),0), pass_matShininess);
	}

	return pass_matSpecular * specular * specularTerm;
}

void main() {

	vec3 B = normalize(cross(pass_Normal, pass_Tangent));
	mat3 M = mat3(pass_Tangent, B, pass_Normal);

	vec3 newNorm = M * Normal;
	
	vec3 L = normalize(pass_LightDir);
	vec3 V = normalize(pass_CameraDir);
	vec3 N = normalize(newNorm);

	vec3 amb = ambientLighting();
	vec3 dif = diffuseLighting(N, L);
	vec3 spe = specularLighting(N, L, V);

	//output color combines all parts (ambient, diffuse, specular)
	out_Color = vec4((amb + dif + spe), 1.0);
	//out_Color = vec4(newNorm, 1.0);
}
