#include "..\h\Buffers.h"

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

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

class Terrain
{
private:
	enum Biome { GRASS, GRASS_DESERT, DESERT, DESERT_PATH, DESERT_OASIS, OASIS };

	// Stores all vertices - triangles across the whole map, with 11 values for each triangle
	// - 3 for vertices, 3 for colours, 3 for normals, 2 for textures
	VAO::VertexData terrainVertices[MAP_SIZE];
	int				normalsCalc[MAP_SIZE];

	bool			trees[MAP_SIZE];

	vector<vec3>	treePositions;

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

	Biome getBiome(float terrain, float path);
	bool getIfModelPlacement(Biome biome, float noise);
public:
	Terrain();

	VAO::VertexData* getVertices();
	ivec3* getIndices();
	void getTreePositions(vector<vec3>* positions);
};