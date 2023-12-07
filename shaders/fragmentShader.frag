// Fragment shader is responsible for mapping colours to the right pixels after
// rasterisation (???)

// Each instance of a running fragment shader is responsible for one pixel & its colour

#version 460

out vec4 FragColor; // Outputted to next stage of graphics pipeline

// Normal coords from vertex shader
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 objColour;
uniform vec3 lightColour;
uniform vec3 viewPos;

in vec3 colourFrag;
in vec2 TexturesFrag;

uniform sampler2D textureInSand;
uniform sampler2D textureInGrass;
uniform sampler2D textureInWater;

// Split texture into 30 tiles so it isn't one large blurred texture
vec2 tiledTex = TexturesFrag * 30;

// Use generated colours as a template for the textures
vec4 textureMap = vec4(colourFrag, 1.0f);

// Distribute textures according to the texture map
vec4 sandTexColour = texture(textureInSand, tiledTex) * textureMap.r;
vec4 grassTexColour = texture(textureInGrass, tiledTex) * textureMap.g;
vec4 waterTexColour = texture(textureInWater, tiledTex) * textureMap.b;

// Get the total colour for this fragment
vec4 totalColour = sandTexColour + grassTexColour + waterTexColour;

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
	//vec3 reflectionDir = reflect(-lightDir, norm); // reflect() expects from light source -> model pos, but we have it the other way so reverse it

	//float specCalc = pow(max(dot(viewDir, reflectionDir), 0.0f), 32); // 32 -> shininess value
	float specCalc = pow(max(dot(norm, halfwayDir), 0.0f), 32); // 32 -> shininess value
	//vec3 specular = specularStrength * specCalc * lightColour;
	vec3 specular = specularStrength * specCalc * lightColour;


	vec3 resultColour = (ambient + diffuse + specular) * objColour;

	// Apply colour
	//FragColor = vec4(colourFrag, 1.0f) * vec4(resultColour, 1.0f);
	//FragColor = texture(textureInSand, tiledTex) * vec4(resultColour, 1.0f);
	FragColor = totalColour * vec4(resultColour, 1.0f);
}