#ifndef TEXTURE_H

#define TEXTURE_H

#include <glad/glad.h>
#include <string>
#include <iostream>

#include <learnopengl/shader_m.h>

#define NUM_TEXTURES	4 // Sand, sand path, grass, water

using namespace std;

// Class for handling textures on the terrain
class TerrainTexture
{
public:
	enum TerrainType { SAND, GRASS, WATER, SAND_2 };
	static const string texNames[NUM_TEXTURES];

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

#endif