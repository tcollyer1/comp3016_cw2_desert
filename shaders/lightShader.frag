#version 460

uniform vec3 lightColour;

out vec4 FragColor; // Outputted to next stage of graphics pipeline

void main()
{
	FragColor = vec4(lightColour, 1.0f);
}