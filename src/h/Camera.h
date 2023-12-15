#include "..\h\Terrain.h"

#define USER_HEIGHT 1.0f

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

class Camera
{
public:
	//typedef void (Camera::*mouseFunc)(GLFWwindow*, double, double);

	struct CameraInfo
	{
		// Position of the camera in world space
		vec3 cameraPos;

		// Travel direction - forward, since we've set it to negative on Z axis, which usually
		// points towards you
		vec3 cameraFront;

		// Absolute up direction
		vec3 cameraUp;
	};

	Camera(Terrain* t);
	CameraInfo getCameraInfo();

	void mouseCallback(GLFWwindow* pW, double x, double y);
	void processUserInput(GLFWwindow* pW, float deltaTime);

private:
	enum CameraMode { FLY, WALK };

	CameraInfo camInfo;
	CameraMode mode;
	Terrain* terrain;

	// If it's the first time the mouse is entering the window, this determines whether to set
	// default last x/y positions or not.
	bool mouseFirstEntry;

	// Camera positions from the last frame
	float cameraLastXPos;
	float cameraLastYPos;

	// Horizontal rotation
	float cameraYaw;

	// Vertical rotation
	float cameraPitch;

	void toggleFly();
	void toggleWalk();
};