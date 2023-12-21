#include "..\h\ShaderInterface.h"

ShaderInterface::ShaderInterface(string v, string f)
{
	shaders = new Shader(v.c_str(), f.c_str());
}

// Sends MVP data to the shaders
void ShaderInterface::setMVP(MVP* mvp)
{
	shaders->use();
	shaders->setMat4("model", mvp->getModel());
	shaders->setMat4("view", mvp->getView());
	shaders->setMat4("projection", mvp->getProjection());
}

// Sends the current light colour to the shaders.
void ShaderInterface::setShaderLightColour(vec3 colour)
{
	shaders->use();
	shaders->setVec3("lightColour", colour);
}

// Sends the current light source and camera view positions to the shaders.
void ShaderInterface::setShaderPositions(vec3 lightPos, vec3 cameraPos)
{
	shaders->use();
	shaders->setVec3("lightPos", lightPos);
	shaders->setVec3("viewPos", cameraPos);
}