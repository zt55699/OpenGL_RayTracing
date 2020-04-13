#version 450 core

uniform float ambientStrength;
uniform vec3 lightColour;
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform float Shininess; 
uniform float Strength; 

in vec3 Normal;
in vec3 FragPos;
in vec3 vertexColour;
in vec3 LightDirection;
in vec3 HalfVector; 
in float Attenuation;

out vec4 fragColour;

void main()
{
	float diffuse = max(0.0, dot(Normal, LightDirection)); 
	float specular = max(0.0, dot(Normal, HalfVector));
					
	if (diffuse == 0.0) 
		specular = 0.0;
	else
		specular = pow(specular, Shininess) * Strength;		

	vec3 scatteredLight = ambientStrength + lightColour * diffuse * Attenuation; 	
	vec3 reflectedLight = lightColour * specular * Attenuation; 
	vec3 rgb = min(vertexColour.rgb * scatteredLight + reflectedLight, vec3(1.0)) ;
	fragColour = vec4(rgb, 1.0);

}