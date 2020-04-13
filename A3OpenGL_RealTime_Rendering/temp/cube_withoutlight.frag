#version 450 core

in vec3 vertexColour;

uniform vec3 lightColour;

out vec4 fragColour;

void main()
{
	fragColour = vec4(lightColour * vertexColour, 1.0);

}