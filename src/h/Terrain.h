#ifndef TERRAIN_H

#define TERRAIN_H

#include "Buffers.h" // Includes GLM
#include "Texture.h" // Includes shader loading
#include "MVP.h"

#include "ShaderInterface.h"

#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include <vector>
#include <string>

// irrKlang - audio
#include <irrKlang/irrKlang.h>

#define TERRAIN_START		vec3(0.0f, -2.0f, -1.5f)

#define RENDER_DIST			512							// Map width/height
#define MAP_SIZE			(RENDER_DIST * RENDER_DIST) // Total map size

#define VERTICE_OFFSET		0.1f						// Distance between each vertice drawn on the terrain

// Global terrain positions
#define START_POS			VERTICE_OFFSET
#define END_POS				(RENDER_DIST * VERTICE_OFFSET)
#define MIDDLE_POS			(END_POS * 0.5)				// Middle coordinate of the map (x & z axes)

// Terrain texture coordinate positions for each corner of the terrain
#define BTM_LEFT			0
#define BTM_RIGHT			(RENDER_DIST - 1)
#define TOP_LEFT			(MAP_SIZE - RENDER_DIST)
#define TOP_RIGHT			(MAP_SIZE - 1)

#define ROW_CHUNKS			(RENDER_DIST - 1) // No. chunks across a single dimension
#define CHUNK_TRIANGLES		2 // Two triangles per square chunk
#define TOTAL_TRIANGLES		(ROW_CHUNKS * ROW_CHUNKS * CHUNK_TRIANGLES) // Total amount of triangles on the map


using namespace std;
using namespace irrklang;

// Class for creating the main terrain object.
class Terrain : public ShaderInterface
{
public:
	enum Biome { GRASS, GRASS_DESERT, DESERT, DESERT_PATH, DESERT_OASIS, OASIS };

	Terrain(string vertexShader, string fragShader) : ShaderInterface(vertexShader, fragShader)
	{
		rowIndex = 0;
		colVerticesOffset = drawStartPos;
		rowVerticesOffset = drawStartPos;
		colIndicesOffset = 0;
		rowIndicesOffset = 0;

		// Generate random model rotations, scaling factors and randomise between
		// tree/grass or cactus/grass models on grassy and oasis biomes
		for (int i = 0; i < MAP_SIZE; i++)
		{
			modelType[i] = rand() % 2;
			rotation[i] = rand() % (180 - -180 + 1) + -180;
			scaling[i] = rand() % (2 - 1 + 1) + 1;
		}

		generateVertices();
		generateLandscape();
		setTextureCoords();
		generateNormals();

		createTerrainVAO();
		setTextures();

		// Set up audio
		engine = createIrrKlangDevice();

		if (!engine)
		{
			cout << "[!] Error setting up irrKlang engine (Terrain.cpp)\n";
		}

		setSoundTree();
	}

	void getGrassModelPositions(vector<vec3>* positions);
	void getOasisModelPositions(vector<vec3>* positions);
	Biome offsetUserPos(vec3* pos);
	bool isAtEdge(vec3 pos);

	//void setShaderPositions(vec3 lightPos, vec3 cameraPos);
	//void setShaderLightColour(vec3 colour);
	//void setMVP(MVP* mvp);

	const int getModelType(int idx);
	const int getRotation(int idx);
	const int getScale(int idx);

	void drawTerrain();

	void updateListenerPosition(vec3 pos, vec3 front);

private:

	const string assetsFolder = "media/";
	const string treeSound = "media/audio/birdSong.mp3";

	ISoundEngine* engine;
	ISound* sound;

	vec3 soundTreeModel;

	//Shader* shaders;

	// Stores all vertices - triangles across the whole map, with 11 values for each triangle
	// - 3 for vertices, 3 for colours, 3 for normals, 2 for textures
	VAO::VertexData terrainVertices[MAP_SIZE];
	int				normalsCalc[MAP_SIZE];

	VAO*			terrainVAO;

	vector<TerrainTexture*> textures;

	int modelType[MAP_SIZE];
	int rotation[MAP_SIZE];
	int scaling[MAP_SIZE];

	vector<vec3>	grassModelPositions;
	vector<vec3>	oasisModelPositions;

	ivec3 terrainIndices[TOTAL_TRIANGLES];

	// For drawing
	const float drawStartPos = START_POS;
	float colVerticesOffset;
	float rowVerticesOffset;

	int colIndicesOffset;
	int rowIndicesOffset;

	// Current chunk in current row being drawn
	int rowIndex;

	void generateVertices();
	void generateLandscape();
	void setTextureCoords();
	void generateNormals();
	void createTerrainVAO();
	void setTextures();

	void setSoundTree();

	Biome getBiome(float terrain, float path);
	bool getIfModelPlacement(Biome biome, float noise);
};

#endif