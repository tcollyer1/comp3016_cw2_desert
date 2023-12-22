#ifndef CAMERA_H

#define CAMERA_H

#include "Terrain.h"

// Offset from the actual height of the terrain so the user is not "crawling"
// along the floor in walking mode
#define USER_HEIGHT 1.0f

// irrKlang - audio
#include <irrKlang/irrKlang.h>

using namespace std;
using namespace glm;
using namespace irrklang;

// Keeps track of the camera position, and additionally handles user input/mouse movement
class Camera
{
public:
	struct CameraInfo
	{
		// Position of the camera in world space
		vec3 cameraPos;

		// Travel direction
		vec3 cameraFront;

		// Absolute up direction
		vec3 cameraUp;
	};

	Camera(Terrain* t);
	~Camera();

	CameraInfo getCameraInfo();

	void mouseCallback(GLFWwindow* pW, double x, double y);
	void processUserInput(GLFWwindow* pW, float deltaTime);

private:
	enum CameraMode { FLY, WALK };

	CameraInfo camInfo;
	CameraMode mode;
	Terrain* terrain;

	ISoundEngine* engine;
	ISound* sound;
	ISound* sound2;
	ISound* sound3;

	const string sandSound = "media/audio/sandStep.mp3";
	const string grassSound = "media/audio/grassStep.mp3";
	const string waterSound = "media/audio/waterStep.mp3";

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

#endif