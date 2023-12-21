#include "..\h\Display.h"

#define WINDOW_WIDTH		glfwGetVideoMode(glfwGetPrimaryMonitor())->width - 100
#define WINDOW_HEIGHT		glfwGetVideoMode(glfwGetPrimaryMonitor())->height - 100

Display::Display(void (*mouseCallback)(GLFWwindow*, double, double), void (*frameBuffSizeCallback)(GLFWwindow*, int, int))
{
	error = false;

	// Initialise GLFW - used for user I/O, creating windows and more using OpenGL
	glfwInit();

	// Create a GLFWwindow window
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Desert", NULL, NULL);

	// Checks for successful window creation
	if (window == NULL)
	{
		cout << "GLFW window could not instantiate\n";
		glfwTerminate();

		error = true;
	}

	// Set cursor to bind to window, hides cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Assign window to current OpenGL context
	glfwMakeContextCurrent(window);

	// Initialise GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialise GLAD\n";

		error = true;
	}

	// Width and height should match that of the window
	// 0, 0 indicates the position of the window
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Enables "depth" in textures
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND); // Enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Assigns frameBuffSizeCallback as the callback function to dynamically handle 
	// window resizing.
	glfwSetFramebufferSizeCallback(window, frameBuffSizeCallback);

	// Set mouse callback for mouse movement
	glfwSetCursorPosCallback(window, mouseCallback);
}

Display::~Display()
{
	free(window);
	glfwTerminate();
}

// Returns the current created GLFW window.
GLFWwindow* Display::getWindow()
{
	return (window);
}

// Returns true if errors were encountered during window creation.
bool Display::checkErrors()
{
	return (error);
}