#version 450 core

struct Material		
{
	vec3 diffuse;
	vec3 specular;
	float Shininess;
};

struct DirLight {
	vec3 diffuse;
	vec3 direction;
	vec3 specular;
};


struct PointLight {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;

uniform	float ConstantAttenuation;
uniform	float LinearAttenuation;
uniform	float QuadraticAttenuation;
uniform vec3 ambientStrength;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
#define NR_POINT_LIGHTS 2
uniform PointLight pointLights[NR_POINT_LIGHTS];


out vec4 color;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight (PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
    // == ======================================
	// Our lighting is set up in 2 phases: directional, point lights 
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == ======================================
	// Phase1: Directional lighting
	vec3 result = CalcDirLight(dirLight, norm, viewDir);
	// Phase 2: Point lights
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	}
	color = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	// Combine results
	vec3 ambient = ambientStrength * vec3(material.diffuse);
	vec3 diffuse = light.diffuse * diff * vec3(material.diffuse);	
	vec3 specular = light.specular * spec * vec3(material.specular);
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight (PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation  = 1.0f / (ConstantAttenuation + LinearAttenuation * distance + QuadraticAttenuation * (distance * distance));
	// Combine results
    vec3 ambient = ambientStrength * vec3(material.diffuse);
    vec3 diffuse = light.diffuse * diff * vec3(material.diffuse);
    vec3 specular = light.specular * spec * vec3(material.specular);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
