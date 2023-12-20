//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include "..\h\Terrain.h"

// Perlin Noise
#include "..\h\FastNoiseLite.h"

#include <math.h>

using namespace glm;

#define GRASS_MODEL_BOUND	0.95f
#define OASIS_MODEL_BOUND	0.99f

Terrain::Terrain()
{
	shaders = new Shader("shaders/terrainShader.vert", "shaders/terrainShader.frag");

	rowIndex			= 0;
	colVerticesOffset	= drawStartPos;
	rowVerticesOffset	= drawStartPos;
	colIndicesOffset	= 0;
	rowIndicesOffset	= 0;

	// Generate random model rotations, scaling factors and randomise between
	// tree/grass or cactus/grass on grassy and oasis biomes
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
}

void Terrain::setMVP(MVP* mvp)
{
	shaders->setMat4("model", mvp->getModel());
	shaders->setMat4("view", mvp->getView());
	shaders->setMat4("projection", mvp->getProjection());
}

void Terrain::drawTerrain()
{
	for (int i = 0; i < textures.size(); i++)
	{
		textures[i]->bindTexture();
	}

	terrainVAO->bind();

	glDrawElements(GL_TRIANGLES, MAP_SIZE * 32, GL_UNSIGNED_INT, 0);

	terrainVAO->unbind();
}

void Terrain::createTerrainVAO()
{
	terrainVAO = new VAO();
	terrainVAO->bind();

	int verticesArrSize = sizeof(terrainVertices);
	int indicesArrSize = sizeof(terrainIndices);

	// Bind terrain vertices and indices to buffers
	terrainVAO->addBuffer(terrainVertices, verticesArrSize, VAO::VERTICES);
	terrainVAO->addBuffer(terrainIndices, indicesArrSize, VAO::INDICES);

	terrainVAO->enableAttribArrays(BUF_VERTICES | BUF_COLOURS | BUF_NORMALS | BUF_TEXTURES);

	terrainVAO->unbind();
}

void Terrain::setTextures()
{
	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		TerrainTexture* tex = new TerrainTexture(assetsFolder + TerrainTexture::texNames[i], (TerrainTexture::TerrainType)i, shaders);
		tex->bindTexture();

		textures.push_back(tex);
	}
}

void Terrain::setShaderPositions(vec3 lightPos, vec3 cameraPos)
{
	shaders->use();
	shaders->setVec3("lightPos", lightPos);
	shaders->setVec3("viewPos", cameraPos);
}

void Terrain::setShaderLightColour(vec3 colour)
{
	shaders->setVec3("lightColour", colour);
}

int Terrain::getModelType(int idx)
{
	return (modelType[idx]);
}

int Terrain::getRotation(int idx)
{
	return (rotation[idx]);
}

int Terrain::getScale(int idx)
{
	return (scaling[idx]);
}

// Gets the biome type at a given point on the terrain given the relevant
// noise values
Terrain::Biome Terrain::getBiome(float terrain, float path)
{
	Biome biome = DESERT;

	// Grassy biome. This is where plants and cacti will also appear.
	if (terrain >= 0.55f)
	{
		biome = GRASS;
	}
	// If the terrain is just below grass biome level - allow the textures
	// to mix between grass/sand
	else if (terrain < 0.55 && terrain >= 0.5)
	{
		biome = GRASS_DESERT;
	}
	// If terrain height is below -0.35, set to oasis
	// (water) biome
	else if (terrain <= -0.35f)
	{
		biome = OASIS;
	}
	// If the terrain is just above water biome level - allow the textures
	// to mix between water/sand
	// Trees will also appear here to flesh out this biome
	else if (terrain > -0.35 && terrain <= -0.3)
	{
		biome = DESERT_OASIS;
	}
	// Generate pathways - alternate sand colour to add interest to main
	// desert biome
	else if (path < 0.2f)
	{
		biome = DESERT_PATH;
	}
	// Main desert (sand) biome.
	else
	{
		biome = DESERT;
	}

	return (biome);
}

// Gets if a model should be placed at a given position, provided
// the biome and generated noise value
bool Terrain::getIfModelPlacement(Biome biome, float noise)
{
	bool model = false;

	switch (biome)
	{
	case GRASS:
		if (noise > GRASS_MODEL_BOUND)
		{
			model = true;
		}
		break;
	case DESERT_OASIS:
		if (noise > OASIS_MODEL_BOUND)
		{
			model = true;
		}
		break;
	default:
		break;
	}

	return (model);
}

void Terrain::getGrassModelPositions(vector<vec3>* positions)
{
	for (int i = 0; i < grassModelPositions.size(); i++)
	{
		positions->push_back(grassModelPositions[i]);
	}
}

void Terrain::getOasisModelPositions(vector<vec3>* positions)
{
	for (int i = 0; i < oasisModelPositions.size(); i++)
	{
		positions->push_back(oasisModelPositions[i]);
	}
}

// Generate all of the vertices and indices for the terrain
void Terrain::generateVertices()
{
	for (int i = 0; i < MAP_SIZE; i++)
	{
		// Generate vertices and colours for each triangle

		// Vertices
		terrainVertices[i].vertices.x = colVerticesOffset;	// x axis: -->
		terrainVertices[i].vertices.y = 0.0f;				// y axis - up ^ direction, so 0 - using Perlin noise for height maps
		terrainVertices[i].vertices.z = rowVerticesOffset;	// z axis - forward direction (looking at terrain at level view)

		// Set normals to 0 for now
		terrainVertices[i].normals.x = 0.0f;
		terrainVertices[i].normals.y = 0.0f;
		terrainVertices[i].normals.z = 0.0f;
		normalsCalc[i] = 0;

		// Move x position for next triangle on the grid - draws L->R >>>
		colVerticesOffset += VERTICE_OFFSET;

		// Increment chunk index within this row
		rowIndex++;

		// If at the end of the row
		if (rowIndex == RENDER_DIST)
		{
			// Reset row index
			rowIndex = 0;

			// Reset x & z positions for the next row

			// Column is reset back to the start
			colVerticesOffset = drawStartPos;
			// Row is decreased (moved backwards on z axis) - move back a row - draws front -> back ^^^
			rowVerticesOffset -= VERTICE_OFFSET;
		}
	}

	rowIndex = 0;

	// Go up in twos - 2 triangles per one chunk
	for (int i = 0; i < TOTAL_TRIANGLES; i += 2)
	{
		// Generate the indices by mapping the above to chunks (1x1 squares)

		terrainIndices[i].x = colIndicesOffset + rowIndicesOffset;					// Top left				 _
		terrainIndices[i].z = RENDER_DIST + colIndicesOffset + rowIndicesOffset;	// Bottom left			|/
		terrainIndices[i].y = 1 + colIndicesOffset + rowIndicesOffset;				// Top right

		terrainIndices[i + 1].x = 1 + colIndicesOffset + rowIndicesOffset;					// Top right		/|
		terrainIndices[i + 1].z = RENDER_DIST + colIndicesOffset + rowIndicesOffset;		// Bottom left		-
		terrainIndices[i + 1].y = 1 + RENDER_DIST + colIndicesOffset + rowIndicesOffset;	// Bottom right

		// Move x position for next triangle on the grid
		colIndicesOffset += 1;

		// Increment chunk index within this row
		rowIndex++;

		// If at the end of the row
		if (rowIndex == ROW_CHUNKS)
		{
			// Reset row index
			rowIndex = 0;

			// Reset x & z positions for the next row

			// Column is reset back to the start
			colIndicesOffset = 0;
			// Move back a row
			rowIndicesOffset = rowIndicesOffset + RENDER_DIST;
		}
	}
}

// Generate height maps for the terrain with Perlin noise,
// as well as create colour map based on biomes as a template for
// the textures for the shader.
void Terrain::generateLandscape()
{
	// Assign perlin noise type for the map. This affects the y axis
	FastNoiseLite terrainNoise;
	terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	// Set noise scale
	terrainNoise.SetFrequency(0.025f);
	// Generate random seed
	int seed = rand() % 100;
	terrainNoise.SetSeed(seed);

	// Perlin noise for pathway map
	FastNoiseLite pathNoise;
	pathNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	pathNoise.SetFrequency(0.05f);
	int pSeed = rand() % 100;
	pathNoise.SetSeed(pSeed);

	// Perlin noise for model placement
	FastNoiseLite modelNoise;
	modelNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	modelNoise.SetFrequency(10.0f);
	int tSeed = rand() % 100;
	modelNoise.SetSeed(tSeed);

	// Terrain vertice index
	int terrainIndex = 0;

	float terrainVal = 0.0f;
	float pathVal = 0.0f;
	float modelsVal = 0.0f;

	Biome currBiome = DESERT;

	for (int x = 0; x < RENDER_DIST; x++)
	{
		for (int y = 0; y < RENDER_DIST; y++)
		{
			// Get noise values for biome type and terrain height (between -1 and 1)
			// at the given x/y coordinate (2D position)

			// Generate noise at 3 different frequencies for additional variation
			terrainVal = 1 * terrainNoise.GetNoise((float)x, (float)y)
				+ 0.5 * terrainNoise.GetNoise(2 * (float)x, 2 * (float)y)
				+ 0.25 * terrainNoise.GetNoise(4 * (float)x, 4 * (float)y);

			// Generate noise and get absolute value (turbulence).
			// This produces a simple noise map that gives the impression of
			// winding pathways
			pathVal = abs(pathNoise.GetNoise((float)x, (float)y));

			modelsVal = modelNoise.GetNoise((float)x, (float)y);

			// Set random height value (random noise) calculated before,
			// to the vertex y value. 
			// Divide by the sum of the 3 amplitudes to maintain values between 0-1 (?)
			// Multiply by 2 for greater height diversity.
			terrainVertices[terrainIndex].vertices.y = (terrainVal / (1 + 0.5 + 0.25)) * 2;

			currBiome = getBiome(terrainVal, pathVal);

			// Grass
			if (currBiome == GRASS)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 1.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;

				// Set grass/cacti here
				if (getIfModelPlacement(currBiome, modelsVal))
				{
					grassModelPositions.push_back(vec3(terrainVertices[terrainIndex].vertices));
				}
			}
			// Grass-desert transition
			else if (currBiome == GRASS_DESERT)
			{
				terrainVertices[terrainIndex].colours.r = 0.5f;
				terrainVertices[terrainIndex].colours.g = 1.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}
			// Water
			else if (currBiome == OASIS)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 1.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}
			// Desert-water transition
			else if (currBiome == DESERT_OASIS)
			{
				terrainVertices[terrainIndex].colours.r = 1.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.5f;
				terrainVertices[terrainIndex].colours.a = 0.0f;

				// Set grass/cacti here
				if (getIfModelPlacement(currBiome, modelsVal))
				{
					oasisModelPositions.push_back(vec3(terrainVertices[terrainIndex].vertices));
				}
			}
			// Sandy pathway
			else if (currBiome == DESERT_PATH)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 1.0f;
			}
			// Normal sand
			else
			{
				terrainVertices[terrainIndex].colours.r = 1.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}

			terrainIndex++;
		}
	}
}

// Calculate the texture coordinates for the terrain object.
void Terrain::setTextureCoords()
{
	// Texture coords - bottom left
	terrainVertices[BTM_LEFT].textures.x = 0.0f;
	terrainVertices[BTM_LEFT].textures.y = 0.0f;

	// Bottom right
	terrainVertices[BTM_RIGHT].textures.x = 1.0f;
	terrainVertices[BTM_RIGHT].textures.y = 0.0f;

	// Top left
	terrainVertices[TOP_LEFT].textures.x = 0.0f;
	terrainVertices[TOP_LEFT].textures.y = 1.0f;

	// Top right
	terrainVertices[TOP_RIGHT].textures.x = 1.0f;
	terrainVertices[TOP_RIGHT].textures.y = 1.0f;

	int z = 0;

	for (int x = 0; x < MAP_SIZE; x++)
	{
		div_t divResultX;
		div_t divResultZ;

		divResultX = div(x, RENDER_DIST);
		divResultZ = div(z, RENDER_DIST);

		if (x != BTM_LEFT
			&& x != BTM_RIGHT
			&& x != TOP_LEFT
			&& x != TOP_RIGHT)
		{
			terrainVertices[x].textures.x = (float)divResultX.rem / RENDER_DIST;
			terrainVertices[x].textures.y = (float)divResultZ.rem / RENDER_DIST;
		}

		if (divResultX.rem == RENDER_DIST - 1)
		{
			z++;
		}
	}
}

// Calculate the normal map for the terrain for BlinnPhong lighting.
void Terrain::generateNormals()
{
	for (int i = RENDER_DIST; i < MAP_SIZE; i++)
	{
		div_t divResult;
		divResult = div(i, RENDER_DIST);

		// If not at the right edge
		if (divResult.rem != RENDER_DIST - 1)
		{
			vec3 topLeft = vec3(terrainVertices[i].vertices);
			vec3 topRight = vec3(terrainVertices[i + 1].vertices);

			vec3 btmLeft = vec3(terrainVertices[i - RENDER_DIST].vertices);

			// Multiply by -1 to ensure lighting doesn't appear reversed
			vec3 normal = normalize(cross((topRight - topLeft), (btmLeft - topLeft)) * -1.0f);

			// Top left of quad
			terrainVertices[i].normals.x += normal.x;
			terrainVertices[i].normals.y += normal.y;
			terrainVertices[i].normals.z += normal.z;
			normalsCalc[i]++;

			// Top right of quad
			terrainVertices[i + 1].normals.x += normal.x;
			terrainVertices[i + 1].normals.y += normal.y;
			terrainVertices[i + 1].normals.z += normal.z;
			normalsCalc[i + 1]++;

			// Bottom left of quad
			terrainVertices[i - RENDER_DIST].normals.x += normal.x;
			terrainVertices[i - RENDER_DIST].normals.y += normal.y;
			terrainVertices[i - RENDER_DIST].normals.z += normal.z;
			normalsCalc[i - RENDER_DIST]++;

			// Bottom right of quad
			terrainVertices[(i - RENDER_DIST) + 1].normals.x += normal.x;
			terrainVertices[(i - RENDER_DIST) + 1].normals.y += normal.y;
			terrainVertices[(i - RENDER_DIST) + 1].normals.z += normal.z;
			normalsCalc[(i - RENDER_DIST) + 1]++;
		}
	}

	// Average normals where a vertice has multiple normal values
	for (int i = RENDER_DIST; i < MAP_SIZE; i++)
	{
		terrainVertices[i].normals.x /= (float)normalsCalc[i];
		terrainVertices[i].normals.y /= (float)normalsCalc[i];
		terrainVertices[i].normals.z /= (float)normalsCalc[i];
	}
}

bool Terrain::isAtEdge(vec3 pos)
{
	bool atEdge = false;
	vec3 terrainCoords;

	terrainCoords.x = pos.x - TERRAIN_START.x;
	terrainCoords.z = pos.z - TERRAIN_START.z;

	if (terrainCoords.x < terrainVertices[BTM_LEFT].vertices.x
		|| terrainCoords.x > terrainVertices[BTM_RIGHT].vertices.x
		|| terrainCoords.z < terrainVertices[TOP_LEFT].vertices.z
		|| terrainCoords.z > terrainVertices[BTM_LEFT].vertices.z)
	{
		atEdge = true;
	}

	return (atEdge);
}

void Terrain::offsetUserPos(vec3* pos)
{
	vec3 terrainCoords;

	terrainCoords.x = pos->x - TERRAIN_START.x;
	terrainCoords.y = pos->y - TERRAIN_START.y;
	terrainCoords.z = pos->z - TERRAIN_START.z;

	bool found = false;
	int i = 0;

	// Get up vector at this terrain vertice
	while (i < MAP_SIZE - 1 && !found)
	{
		if ((terrainVertices[i].vertices.x == terrainCoords.x || (terrainCoords.x < terrainVertices[i].vertices.x + VERTICE_OFFSET && terrainCoords.x > terrainVertices[i].vertices.x - VERTICE_OFFSET))
			&& (terrainVertices[i].vertices.z == terrainCoords.z || (terrainCoords.z < terrainVertices[i].vertices.z + VERTICE_OFFSET && terrainCoords.z > terrainVertices[i].vertices.z - VERTICE_OFFSET)))
		{
			//float diffX = terrainVertices[i].vertices.x - terrainCoords.x;
			//float diffZ = terrainVertices[i].vertices.z - terrainCoords.z;
			//float crossVal = cross(vec3(diffX, 0.0f, 0.0f), vec3(0.0f, 0.0f, diffZ)).y;

			terrainCoords.y = terrainVertices[i].vertices.y;// + crossVal;
			found = true;
		}

		i++;
	}

	pos->y = terrainCoords.y;	
}