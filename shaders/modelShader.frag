#version 460

out vec4 FragColor; // Outputted to next stage of graphics pipeline

in vec2 TexturesFrag;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos;
vec3 objColour = vec3(1.0f);
uniform vec3 lightColour;
uniform vec3 viewPos;

void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);

	// AMBIENT
	float ambientStr = 0.1f;
	vec3 ambient = ambientStr * lightColour;

	// DIFFUSE
	float difference = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = difference * lightColour;

	// SPECULAR
	float specularStrength = 0.5f;
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 halfwayDir = normalize(lightDir + viewDir);

	float specCalc = pow(max(dot(norm, halfwayDir), 0.0f), 32); // 32 -> shininess value
	vec3 specular = specularStrength * specCalc * lightColour;

	// Calculate the resulting colour to apply
	vec3 resultColour = (ambient + diffuse + specular) * objColour;

	// Apply colour - texture multiplied by lighting effect
	FragColor = texture(texture_diffuse1, TexturesFrag) * vec4(resultColour, 1.0f);
}