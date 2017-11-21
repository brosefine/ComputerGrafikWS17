#version 150

in vec3 pass_Normal;
in vec3 pass_LightDir;
in vec3 pass_CameraDir;

in vec3 pass_Color;

out vec4 out_Color;

//calculate diffuse factor
float diffuseLighting(in vec3 N, in vec3 L){

	return clamp(dot(L,N),0.0,1.0);

}

//calculate specular factor
float specularLighting(in vec3 N, in vec3 L, in vec3 V){
	
	if(dot(N,L)>0){
		vec3 H = normalize(L+V);
      return pow(clamp(dot(H,N),0.0,1.0),64.0);
   	}
   	return 0.0;
}

void main() {
	
	//thickness of outline
	float outline = 0.35;
	//number of shade colors
	float shades = 3;
	
	vec3 L = normalize(pass_LightDir);
	vec3 V = normalize(pass_CameraDir);
	vec3 N = normalize(pass_Normal);

	//angle between normal and view direction
	float dotView = dot(pass_Normal, pass_CameraDir);

	if(dotView < outline){
		//color outline pixels
		out_Color = vec4(pass_Color, 1.0);
	} else {
	
		//ambient factor
		float amb = 0.1;
		float dif = diffuseLighting(N, L);
		float spe = specularLighting(N, L, V);

		float intensity = amb + dif + spe;
		float shadeIntensity = ceil(intensity * shades)/shades;
	
	
		out_Color = vec4((pass_Color * shadeIntensity), 1.0);
	}

}
