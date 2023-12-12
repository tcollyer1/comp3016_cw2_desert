#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

// Display class will be responsible for managing the main GLFW window and its interactions.
class Display
{
private:
	GLFWwindow* window;
	bool error;

public:

	Display(void (*mouseCallback)(GLFWwindow*, double, double), void (*frameBuffSizeCallback)(GLFWwindow*, int, int));

	GLFWwindow*		getWindow();
	bool			checkErrors();
};