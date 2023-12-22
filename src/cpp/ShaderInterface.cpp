#include "..\h\ShaderInterface.h"

ShaderInterface::ShaderInterface(string v, string f, int* err)
{
	(*err) = 0;

	fstream vert(v);
	fstream frag(f);

	if (!vert.good() || !frag.good())
	{
		(*err) = 1;
		shaders = NULL;
	}
	else
	{
		shaders = new Shader(v.c_str(), f.c_str());
	}	
}

ShaderInterface::~ShaderInterface()
{
	glUseProgram(0);
	free(shaders);
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