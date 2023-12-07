// Language in use is GLSL - similar to C++
// Vertex shader determines all the positions of the vertices of an object

#version 460 // Establish GLSL version
 
// Position with values retrieved from main.cpp
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoords;

// Normal out, fragment position out
out vec3 Normal;
out vec3 FragPos;
out vec2 TexturesFrag;

// Uniform variable for MVP (model view projection) matrix
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	// Vertices sent through gl_Position to the next stage. Any transforms applied by multiplying
	gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);

	TexturesFrag = textureCoords;

	FragPos = vec3(model * vec4(position, 1.0f));
	Normal = mat3(transpose(inverse(model))) * normal;
}