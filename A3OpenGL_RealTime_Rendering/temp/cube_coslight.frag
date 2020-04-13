#version 450 core
in vec3 Normal;
in vec3 FragPos;
in vec3 vertexColour;

uniform float ambientStrength;
uniform vec3 lightColour;
uniform vec3 lightPos;
uniform vec3 objectColor;

out vec4 fragColour;

void main()
{
	vec3 ambient = ambientStrength * lightColour;

	vec3 norm = normalize(Normal);						
	vec3 lightDir = normalize(lightPos - FragPos);	
	float diff = max(dot(norm, lightDir), 0.0);		
	vec3 diffuse = diff * lightColour;	

	vec3 result = (ambient + diffuse) * objectColor;
	fragColour = vec4(result, 1.0);

}