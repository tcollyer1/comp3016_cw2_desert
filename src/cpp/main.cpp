#include <iostream>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shaders
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Other
#include <math.h>
#include <vector>

#include "..\h\main.h"
#include "..\h\Display.h"
#include "..\h\Buffers.h"
#include "..\h\Terrain.h"
#include "..\h\Light.h"
#include "..\h\Texture.h"
#include "..\h\Camera.h"

using namespace std;
using namespace glm;

/*******************************************************************************************
*
* MATRIX GLOBALS
*
*******************************************************************************************/
// Global matrices for transformations
mat4 transform;
mat4 model;
mat4 view;
mat4 projection;

vec3 rockPos = vec3(0.0f);
int treeOrShrub[MAP_SIZE];

/*******************************************************************************************
*
* TIME GLOBALS
*
*******************************************************************************************/
// Time diff - time taken between each frame change
float deltaTime = 0.0f;
// Last value of time diff - last current frame
float lastFrame = 0.0f;



/*******************************************************************************************
*
* CAMERA
*
*******************************************************************************************/
Camera* camera = new Camera();

int main()
{
	Display* d = new Display(mouseCallback, frameBufferSizeCallback);

	if (d->checkErrors())
	{
		cout << "\nErrors found: closing program.\n";

		return -1;
	}

	// Set shaders & model
	Shader terrainShaders("shaders/terrainShader.vert", "shaders/terrainShader.frag");
	Shader lightShaders("shaders/lightShader.vert", "shaders/lightShader.frag");
	Shader modelShaders("shaders/modelShader.vert", "shaders/modelShader.frag");

	Model rock("media/rock/Rock07-Base.obj");
	Model shrub("media/shrub/free grass by adam127.obj"); // TODO: Model needs replacing, low quality
	Model palmTree("media/palmTree/CordylineFREE.obj");

	/*******************************************************************************************
	*
	* CREATING THE TERRAIN
	*
	*******************************************************************************************/

	Terrain* terrain = new Terrain();

	VAO* terrainVAO = new VAO();
	terrainVAO->bind();

	int verticesArrSize = sizeof(*terrain->getVertices()) * MAP_SIZE;
	int indicesArrSize = sizeof(*terrain->getIndices()) * TOTAL_TRIANGLES;

	// Bind terrain vertices and indices to buffers
	terrainVAO->addBuffer(terrain->getVertices(), verticesArrSize, VAO::VERTICES);
	terrainVAO->addBuffer(terrain->getIndices(), indicesArrSize, VAO::INDICES);

	terrainVAO->enableAttribArrays();

	terrainVAO->unbind();

	/*******************************************************************************************
	*
	* TERRAIN TEXTURES
	*
	*******************************************************************************************/

	//-------------------------
	// TEXTURE 1: SAND
	//-------------------------
	TerrainTexture* sand = new TerrainTexture("media/sand.jpg", TerrainTexture::SAND, &terrainShaders);
	sand->bindTexture();

	//-------------------------
	// TEXTURE 2: GRASS
	//-------------------------

	TerrainTexture* grass = new TerrainTexture("media/grass.jpg", TerrainTexture::GRASS, &terrainShaders);
	grass->bindTexture();

	//-------------------------
	// TEXTURE 3: WATER
	//-------------------------

	TerrainTexture* water = new TerrainTexture("media/water.jpg", TerrainTexture::WATER, &terrainShaders);
	water->bindTexture();

	//-------------------------
	// TEXTURE 4: SAND 2 (path)
	//-------------------------

	TerrainTexture* sand2 = new TerrainTexture("media/sandPath.jpg", TerrainTexture::SAND_2, &terrainShaders);
	sand2->bindTexture();

	/*******************************************************************************************
	*
	* CREATING LIGHT SOURCE
	*
	*******************************************************************************************/
	VAO* testLightVAO = new VAO();
	testLightVAO->bind();

	Light* light = new Light();

	verticesArrSize = sizeof(*light->getVertices()) * NUM_LIGHT_VERTICES;
	testLightVAO->addBuffer(light->getVertices(), verticesArrSize, VAO::VERTICES);

	testLightVAO->enableAttribArrays();

	testLightVAO->unbind();


	// TODO: Generate list of all positions where trees must be drawn
	vector<vec3> treePositions;
	terrain->getTreePositions(&treePositions);

	// Randomise whether tree or shrub is placed in grassy areas
	for (int i = 0; i < treePositions.size(); i++)
	{
		treeOrShrub[i] = rand() % 2;
	}


	/*******************************************************************************************
	* 
	*	**** RENDER LOOP ****
	*	---------------------
	* 
	*******************************************************************************************/

	Camera::CameraInfo camInfo;

	// This is the render loop. glfwWindowShouldClose() is always false (by default)
	// until the 'X' is clicked on the window. It can also manually be changed, see 
	// ProcessUserInput().

	while (!glfwWindowShouldClose(d->getWindow()))
	{
		/////////////////////////////////////////////////////////////////////////////////////
		// *** Time & Frames *** //
		// --------------------- //
		float currFrame = static_cast<float>(glfwGetTime());
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;

		// Set current camera/light position for specular lighting
		terrainShaders.use();
		terrainShaders.setVec3("lightPos", light->getLightPosition());
		terrainShaders.setVec3("viewPos", camera->getCameraInfo().cameraPos);


		/////////////////////////////////////////////////////////////////////////////////////
		// *** Process user input *** //
		// -------------------------- //
		camera->processUserInput(d->getWindow(), deltaTime);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Render *** //
		// -------------- //
		// The colour to be displayed on the cleared window (RGBA)

		glClearColor(light->getSkyColour().r, light->getSkyColour().g, light->getSkyColour().b, 1.0f);

		// Clears the colour buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Transform *** //
		// ----------------- //
		// Dynamic transformations must be done in the rendering loop for constant updates.

		/*** (Model-view-projection) ***/

		// Set our view for the MVP here.
		// Initialise the view & relative positioning.
		camInfo = camera->getCameraInfo();

		view = lookAt(camInfo.cameraPos, camInfo.cameraPos + camInfo.cameraFront, camInfo.cameraUp);
		projection = perspective(radians(45.0f), (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width / (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height, 0.1f, 100.0f);

		// Create matrix, same as transform before
		model = mat4(1.0f);

		model = translate(model, TERRAIN_START);

		// Set our MVP matrix, mvp, to the uniform variable
		terrainShaders.setMat4("model", model);
		terrainShaders.setMat4("view", view);
		terrainShaders.setMat4("projection", projection);

		// Set value of model colour and light colour
		terrainShaders.setVec3("lightColour", light->getLightColour());

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw *** //
		// ------------ //

		// Draw terrain
		sand->bindTexture();
		grass->bindTexture();
		water->bindTexture();
		sand2->bindTexture();
		terrainVAO->bind();

		glDrawElements(GL_TRIANGLES, MAP_SIZE * 32, GL_UNSIGNED_INT, 0);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw Rock *** //
		// ----------------- //;
		// Create matrix, same as transform before
		modelShaders.use();


		modelShaders.setVec3("lightPos", light->getLightPosition());
		modelShaders.setVec3("lightColour", light->getLightColour());

		for (int i = 0; i < treePositions.size(); i++)
		{
			model = mat4(1.0f);

			rockPos.x = TERRAIN_START.x + treePositions[i].x;
			rockPos.y = TERRAIN_START.y + treePositions[i].y;
			rockPos.z = TERRAIN_START.z + treePositions[i].z;

			model = translate(model, rockPos);

			// Set our MVP matrix to the uniform variables
			modelShaders.setMat4("view", view);
			modelShaders.setMat4("projection", projection);
			modelShaders.setVec3("viewPos", camInfo.cameraPos);

			// Draw model using enabled shaders
			if (treeOrShrub[i])
			{
				// Scale down tree object, draw tree
				model = scale(model, vec3(0.005f, 0.005f, 0.005f));

				modelShaders.setMat4("model", model);

				palmTree.Draw(modelShaders);
			}
			else
			{
				// Scale down grass object, draw grass
				model = scale(model, vec3(0.2f, 0.2f, 0.2f));

				modelShaders.setMat4("model", model);

				shrub.Draw(modelShaders);
			}
			
		}

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw the Light Cube *** //
		// --------------------------- //
		// Switch to light cube shaders, bind other VAO
		//glUseProgram(lightProg);
		lightShaders.use();
		testLightVAO->bind();

		lightShaders.setVec3("lightColour", light->getLightColour());

		model = mat4(1.0f);

		// Move model
		model = translate(model, light->getLightPosition());

		// Set our MVP matrix to the uniform variables
		lightShaders.setMat4("model", model);
		lightShaders.setMat4("view", view);
		lightShaders.setMat4("projection", projection);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Move light position for next time
		light->moveLight(glfwGetTime());

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Refresh *** //
		// --------------- //
		// Constantly swap colour buffer being rendered to the window.
		//	- The colour buffer is a 2D buffer containing colour values per pixel of
		//	  the window it's being rendered to.
		glfwSwapBuffers(d->getWindow());

		// Constantly query whether any GLFW events have been triggered - such as
		// the frameBufferSizeCallback() function
		glfwPollEvents();
	}

	/*******************************************************************************************
	*
	* TERMINATE
	*
	*******************************************************************************************/

	// Safely terminate GLFW
	glfwTerminate();

	return 0;
}

// Callback function to handle mouse movement.
void mouseCallback(GLFWwindow* pW, double x, double y)
{
	if (camera != NULL)
	{
		camera->mouseCallback(pW, x, y);
	}
	else
	{
		cout << "ERROR: camera is null\n";
	}
}

// Callback function to adjust the window width/height for window resizing.
void frameBufferSizeCallback(GLFWwindow* pW, int width, int height)
{
	glViewport(0, 0, width, height);
}

