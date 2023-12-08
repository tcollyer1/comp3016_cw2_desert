#include "..\h\Texture.h"

// Textures - header must be in C:/Users/Public/OpenGL/include
#include "stb_image.h"

TerrainTexture::TerrainTexture(string path, TerrainType type, Shader* terrainShader)
{
	this->path		= path;
	data			= NULL;
	width			= 0;
	height			= 0;
	colourChannels	= 0;

	switch (type)
	{
	case SAND:
	case GRASS:
	case WATER:
		textureSlot = GL_TEXTURE0 + (int)type;
		break;
	default:
		cout << "\nERROR: Invalid texture slot (" << type << ") selected for texture at path " << path << "\n";
		textureSlot = 0;
	}

	// Assign texture to generate and bind to 2D texture
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	// Selects x axis of texture bound to GL_TEXTURE_2D & sets to repeat beyond 
	// normalised coordinates (WRAP_S)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// Selects y axis and does the same (WRAP_T)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Texture scaling/filtering.
	// Sets to use linear interpolation between adjacent mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Sets to use linear interpolation upscaling (past largest mipmap texture) (default upscaling method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load(path.c_str(), &width, &height, &colourChannels, 4); // TODO: Back to 0 if it's weird

	if (data) // If retrieval was successful
	{
		// Generation of texture from retrieved texture data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // TODO: Switch to RGB if weird
		// Automatically generates all required mipmaps on bound texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else // If retrieval unsuccessful
	{
		cout << "ERROR: Failed to load texture at path " << path << ".\n";
	}

	// Clears retrieved texture from memory
	stbi_image_free(data);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Enable shader and set uniform variable for this terrain texture
	terrainShader->use();
	terrainShader->setInt("texture" + to_string((int)type), type);
	// Disable shader program
	glUseProgram(0);
}

TerrainTexture::~TerrainTexture()
{
	glDeleteTextures(1, &id);
}

void TerrainTexture::bindTexture()
{
	// Set active texture first - every texture after GL_TEXTURE0 is consecutive
	glActiveTexture(textureSlot);

	glBindTexture(GL_TEXTURE_2D, id);	
}

void TerrainTexture::unbindTexture()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}