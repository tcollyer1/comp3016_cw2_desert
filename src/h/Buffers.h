#ifndef BUFFERS_H

#define BUFFERS_H

#include <glad/glad.h>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class VAO;
class VBO;
class IBO;

class VBO
{
protected:
	GLuint bufferId;

	VBO(const void* pData, int size);

	void bind();
	void unbind();

	friend VAO;
};

class IBO
{
private:
	GLuint bufferId;
	int idxCount;

protected:
	IBO(const void* pData, int size);

	void bind();
	void unbind();

	int getCount();

	friend VAO;
};

class VAO
{
public:
	struct VertexData
	{
		vec3 vertices;
		vec4 colours;
		vec3 normals;
		vec2 textures;
	};

	enum BufferType { VERTICES, NORMALS, TEXTURES, COLOURS, NUM_ATTRIBS, INDICES };

	VAO();
	~VAO();

	void bind();
	void unbind();

	void enableAttribArrays();

	GLuint getVBOId();
	GLuint getIBOId();

	void addBuffer(const void* pData, int size, BufferType type);

private:
	GLuint vaoId;
	VBO* verticesBuffer;
	IBO* indicesBuffer;

	int getCoordSize(BufferType type);
};

#endif