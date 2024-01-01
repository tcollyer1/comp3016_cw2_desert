#include "..\h\Terrain.h"

// Noise - height maps, model placement
#include "..\h\FastNoiseLite.h"

#include <math.h>

using namespace glm;

// Defines the boundaries the noise values must exceed
// for a model to be placed in the grass & oasis biomes
#define GRASS_MODEL_BOUND	0.95f
#define OASIS_MODEL_BOUND	0.99f

Terrain::~Terrain()
{
	if (engine)
	{
		engine->drop();
	}
	if (sound)
	{
		sound->stop();
		sound->drop();
	}

	free(sound);
	free(engine);
	free(terrainVAO);

	glUseProgram(0);
	free(shaders);
}

// Sets the tree that will have a 3D sound attached to it
void Terrain::setSoundTree()
{
	int i = 0;
	bool found = false;

	// Search for a tree and use its position for the 3D sound
	while (i < oasisModelPositions.size() && !found)
	{
		if (modelType[i])
		{
			soundTreeModel = vec3(oasisModelPositions[i]);
			found = true;
		}

		i++;
	}

	if (engine && soundTreeModel.x != 0.0f && soundTreeModel.y != 0.0f && soundTreeModel.z != 0.0f)
	{
		sound = engine->play3D(treeSound.c_str(), vec3df(soundTreeModel.x, soundTreeModel.y, soundTreeModel.z), true, false, true);
		sound->setMinDistance(2.0f);
	}	
}

// Updates listener position, i.e. current camera position, for 3D sound effects
void Terrain::updateListenerPosition(vec3 pos, vec3 front)
{
	if (engine)
	{
		engine->setListenerPosition(vec3df(pos.x, pos.y, pos.z), vec3df(front.x, front.y, front.z));
	}	
}

// Draws the terrain data within the terrain VAO
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

// Sets up VAO for terrain data, including vertices, colours, normals
// and textures
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

// Generates and binds all the textures needed for the terrain.
void Terrain::setTextures()
{
	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		TerrainTexture* tex = new TerrainTexture(assetsFolder + TerrainTexture::texNames[i], (TerrainTexture::TerrainType)i, shaders);
		tex->bindTexture();

		textures.push_back(tex);
	}
}

// Returns either 0 (grass model) or 1 (tree or cactus model, depending on the biome) 
// to determine a randomised model at a given position.
const int Terrain::getModelType(int idx)
{
	return (modelType[idx]);
}

// Retrieves a rotation value from a selection of randomly generated values to apply
// to a given model.
const int Terrain::getRotation(int idx)
{
	return (rotation[idx]);
}

// Retrieves a scaling factor (divisor) from a selection of randomly generated values
// to apply to a given model.
const int Terrain::getScale(int idx)
{
	return (scaling[idx]);
}

// Gets the biome type at a given point on the terrain given the relevant
// noise values
Terrain::Biome Terrain::getBiome(float terrain, float path)
{
	Biome biome = DESERT;

	// Grassy biome. This is where grass and cacti will also appear.
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
	// Trees will also appear here to surround the water and flesh out the
	// oasis biome
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
	bool placeModel = false;

	switch (biome)
	{
	case GRASS:
		if (noise > GRASS_MODEL_BOUND)
		{
			placeModel = true;
		}
		break;
	case DESERT_OASIS:
		if (noise > OASIS_MODEL_BOUND)
		{
			placeModel = true;
		}
		break;
	default:
		break;
	}

	return (placeModel);
}

// Retrieves all of the established model positions for the grassy
// biomes and copies them into the provided vector.
void Terrain::getGrassModelPositions(vector<vec3>* positions)
{
	for (int i = 0; i < grassModelPositions.size(); i++)
	{
		positions->push_back(grassModelPositions[i]);
	}
}

// Retrieves all of the established model positions for the oasis
// biomes and copies them into the provided vector.
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

	// OpenSimplex noise for model placement - OS seems to give more "extreme" values closer together -
	// when tested on terrain, there were considerably more hills and troughs, with less in between values.
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

			modelsVal = modelNoise.GetNoise((float)x, (float)y); // Generate noise for model placement

			// Set random height value (random noise) calculated before,
			// to the vertex y value. 
			// Divide by the sum of the 3 amplitudes to maintain values between 0-1
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

	// Scale 2D texture coordinates across the terrain between 0.0 and 1.0.
	// Treat the terrain as one large quad
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

		// Move onward a row when at the end
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
			// Splits each coordinate into being part of a quad - 
			// uses neighbouring vertices
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

// Determines if a given position is at the boundary of the terrain.
// Used to ensure the user cannot walk off the map
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

// Returns the current biome at a given position (for audio purposes), 
// as well as updating the given camera position's y coordinate with 
// the y coordinate of the terrain at this x/z position.
Terrain::Biome Terrain::offsetUserPos(vec3* pos)
{
	vec3 terrainCoords;
	vec4 biomeColours;

	Biome b;

	// Terrain coordinates are the global camera coordinates, minus
	// the terrain start values
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
			terrainCoords.y = terrainVertices[i].vertices.y;
			biomeColours = terrainVertices[i].colours;

			found = true;
		}

		i++;
	}

	// Uses terrain colour map to determine biome.
	// - Grass-desert transition is considered grass
	// - Desert path is considered desert
	// - Desert-oasis transition is considered oasis
	if (biomeColours.r == 1.0f || biomeColours.a == 1.0f)
	{
		b = DESERT;
	}
	else if (biomeColours.g == 1.0f)
	{
		b = GRASS;
	}
	else
	{
		b = OASIS;
	}

	pos->y = terrainCoords.y;

	return (b);
}