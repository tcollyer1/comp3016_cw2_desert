#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoords;
layout (location = 3) in vec4 colour;

out vec4 colourFrag;

// Send out normals, fragment position and textures to fragment shader
out vec3 Normal;
out vec3 FragPos;
out vec2 TexturesFrag;

// Uniform variable for MVP matrix
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);

	colourFrag = colour;
	TexturesFrag = textureCoords;

	FragPos = vec3(model * vec4(position, 1.0f));
	Normal = mat3(transpose(inverse(model))) * normal;
}