#include <glad/glad.h>
#include <string>
#include <iostream>


#include <learnopengl/shader_m.h>

using namespace std;

class TerrainTexture
{
public:
	enum TerrainType { SAND, GRASS, WATER };

	TerrainTexture(string path, TerrainType type, Shader* terrainShader);
	~TerrainTexture();

	void bindTexture();
	void unbindTexture();

private:
	GLuint id;
	string path;
	unsigned char* data;
	int width, height, colourChannels;
	int textureSlot;
};