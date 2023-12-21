#ifndef SHADERINTERFACE_H

#define SHADERINTERFACE_H

// Shaders
#include <learnopengl/shader_m.h>

#include "MVP.h" // Includes GLM
#include <string>

using namespace std;

// Base class for objects that need to interact with shaders
// (light, models, terrain)
class ShaderInterface
{
protected:
	Shader* shaders;

public:
	ShaderInterface(string v, string f);
	~ShaderInterface();

	virtual void setShaderPositions(vec3 lightPos, vec3 cameraPos);
	virtual void setShaderLightColour(vec3 colour);
	virtual void setMVP(MVP* mvp);
};

#endif