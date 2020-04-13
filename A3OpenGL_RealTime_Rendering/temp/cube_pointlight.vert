#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 LightPosition;
uniform vec3 EyeDirection; 
uniform float ConstantAttenuation; 
uniform float LinearAttenuation; 
uniform float QuadraticAttenuation;

out vec3 vertexColour;
out vec3 Normal;
out vec3 FragPos;
out vec3 LightDirection;
out vec3 HalfVector; 
out float Attenuation;

void main()
{
    LightDirection = LightPosition - position; 
    float lightDistance = length(LightDirection);
    LightDirection = LightDirection / lightDistance;
    Attenuation = 1.0 / (ConstantAttenuation + LinearAttenuation * lightDistance + QuadraticAttenuation * lightDistance * lightDistance);
    HalfVector = normalize(LightDirection + EyeDirection);

    gl_Position =proj * view * model * vec4(position, 1.0);
    vertexColour = colour;	
    Normal = normal;
    FragPos = vec3(model * vec4(position, 1.0f));
}
