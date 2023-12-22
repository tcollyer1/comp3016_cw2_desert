#include <iostream>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other
#include <math.h>
#include <vector>

#include "..\h\main.h"
#include "..\h\Display.h"
#include "..\h\Terrain.h"
#include "..\h\Light.h"
#include "..\h\Camera.h"
#include "..\h\MVP.h"
#include "..\h\ModelSet.h"

using namespace std;
using namespace glm;

// Time diff - time taken between each frame change
float deltaTime = 0.0f;
// Last value of time diff - last current frame
float lastFrame = 0.0f;

// Shader paths
const string tVertexShader = "shaders/terrainShader.vert";
const string tFragShader = "shaders/terrainShader.frag";
const string mVertexShader = "shaders/modelShader.vert";
const string mFragShader = "shaders/modelShader.frag";
const string lVertexShader = "shaders/lightShader.vert";
const string lFragShader = "shaders/lightShader.frag";

// Create camera
Camera* camera = NULL;

int main()
{
	srand(time((time_t*)NULL)); // seed rand() with the time to improve the "true" randomness

	// Create and set up GLFW window
	Display* d = new Display(mouseCallback, frameBufferSizeCallback);

	if (d->checkErrors())
	{
		cout << "Errors found: closing program.\n";

		return -1;
	}

	int shaderError;

	// Add terrain, light and models.
	// Close the program (-1) if shaders cannot be loaded.
	Terrain* terrain = new Terrain(tVertexShader, tFragShader, &shaderError);

	if (shaderError)
	{
		cout << "ERROR: Shader loading failed\n";
		return -1;
	}

	Light* light = new Light(lVertexShader, lFragShader, &shaderError);

	if (shaderError)
	{
		cout << "ERROR: Shader loading failed\n";
		return -1;
	}

	ModelSet* models = new ModelSet(terrain, mVertexShader, mFragShader, &shaderError);

	if (shaderError)
	{
		cout << "ERROR: Shader loading failed\n";
		return -1;
	}

	camera = new Camera(terrain);

	/*******************************************************************************************
	* 
	*	**** RENDER LOOP ****
	*	---------------------
	* 
	*******************************************************************************************/

	// For storing camera position information during render loop
	Camera::CameraInfo camInfo;

	MVP* mvp = new MVP(); // Create MVP matrix

	while (!glfwWindowShouldClose(d->getWindow()))
	{
		/////////////////////////////////////////////////////////////////////////////////////
		// *** Time & Frames *** //
		// --------------------- //
		float currFrame = static_cast<float>(glfwGetTime());
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;

		// Set current camera/light position for specular lighting
		terrain->setShaderPositions(light->getLightPosition(), camera->getCameraInfo().cameraPos);


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

		// Set our view/projection for the MVP.
		// Initialise the view & relative positioning.
		camInfo = camera->getCameraInfo();

		mvp->updateView(camInfo.cameraPos, camInfo.cameraPos + camInfo.cameraFront, camInfo.cameraUp);

		mvp->setProjection();

		// Set up model
		mvp->resetModel();
		mvp->moveModel(TERRAIN_START);

		// Send our MVP matrix, mvp, to the terrain shaders
		terrain->setMVP(mvp);

		// Send light colour to the terrain shaders
		terrain->setShaderLightColour(light->getLightColour());

		// Draw terrain
		terrain->drawTerrain();

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw Models *** //
		// ------------------- //

		models->setShaderPositions(light->getLightPosition(), camInfo.cameraPos);
		models->setShaderLightColour(light->getLightColour());

		models->drawModels(mvp);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw the Light Cube *** //
		// --------------------------- //
		
		// Send the current light colour to the light cube shaders
		light->setShaderLightColour(light->getLightColour());

		mvp->resetModel();

		// Move model
		mvp->moveModel(light->getLightPosition());

		// Set our MVP matrix to the uniform variables
		light->setMVP(mvp);

		// Draw
		light->drawLight();

		// Move light position for next time
		light->moveLight(glfwGetTime());

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Refresh *** //
		// --------------- //
		// Constantly swap colour buffer being rendered to the window.
		//	- The colour buffer is a 2D buffer containing colour values per pixel of
		//	  the window it's being rendered to.
		glfwSwapBuffers(d->getWindow());

		// Constantly query whether any GLFW events have been triggered
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

// Wrapper callback function.
// Calls the camera's callback to handle mouse movement
void mouseCallback(GLFWwindow* pW, double x, double y)
{
	if (camera != NULL)
	{
		camera->mouseCallback(pW, x, y);
	}
	else
	{
		cout << "ERROR: camera not initialised\n";
	}
}

// Callback function to adjust the window width/height for window resizing.
void frameBufferSizeCallback(GLFWwindow* pW, int width, int height)
{
	glViewport(0, 0, width, height);
}