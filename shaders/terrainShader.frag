#version 460

out vec4 FragColor; // Outputted to next stage of graphics pipeline

// Normal coords from vertex shader
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 objColour;
uniform vec3 lightColour;
uniform vec3 viewPos;

in vec4 colourFrag;
in vec2 TexturesFrag;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

// Split texture into 30 tiles so it isn't one large blurred texture
vec2 tiledTex = TexturesFrag * 30;

// Use generated colours as a template for the textures
vec4 textureMap = vec4(colourFrag);

// Distribute textures according to the texture map
vec4 sandTexColour = texture(texture0, tiledTex) * textureMap.r;
vec4 grassTexColour = texture(texture1, tiledTex) * textureMap.g;
vec4 waterTexColour = texture(texture2, tiledTex) * textureMap.b;
vec4 grass2TexColour = texture(texture3, tiledTex) * textureMap.a;

// Get the total colour for this fragment
vec4 totalColour = sandTexColour + grassTexColour + waterTexColour + grass2TexColour;

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


	vec3 resultColour = (ambient + diffuse + specular) * objColour;

	// Apply colour
	FragColor = totalColour * vec4(resultColour, 1.0f);
}