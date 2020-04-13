#version 450 core
out vec4 fragColour;

uniform vec3 lightColor;

void main()
{
	fragColour = vec4(lightColor,1.0);

}