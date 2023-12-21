#include "..\h\MVP.h"


MVP::MVP()
{
	model = mat4(1.0f);
	view = mat4(1.0f);
	projection = mat4(1.0f);
	transform = mat4(1.0f);
}

// Sets the model matrix back to a "default" state for new positioning
void MVP::resetModel()
{
	model = mat4(1.0f);
}

// Updates the MVP view
void MVP::updateView(vec3 eye, vec3 centre, vec3 up)
{
	view = lookAt(eye, centre, up);
}

// Updates the MVP projection
void MVP::setProjection()
{
	projection = perspective(radians(45.0f), (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width / (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height, 0.1f, 100.0f);
}

// Moves the model to the desired position
void MVP::moveModel(vec3 position)
{
	model = translate(model, position);
}

// Scales the model by a desired scaling factor
void MVP::scaleModel(vec3 scalingFactor)
{
	model = scale(model, scalingFactor);
}

// Rotates the model, given the rotation angle in degrees
// and axis/axes to rotate on
void MVP::rotateModel(float degrees, vec3 rotationVec)
{
	model = rotate(model, radians(degrees), rotationVec);
}

mat4 MVP::getModel()
{
	return (model);
}

mat4 MVP::getView()
{
	return (view);
}

mat4 MVP::getProjection()
{
	return (projection);
}