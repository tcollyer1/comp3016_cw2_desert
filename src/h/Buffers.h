#ifndef BUFFERS_H

#define BUFFERS_H

#include <glad/glad.h>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BUF_VERTICES 1
#define BUF_NORMALS 2
#define BUF_TEXTURES 4
#define BUF_COLOURS 8

using namespace glm;


class VAO;
class VBO;
class IBO;

// Class for creating and managing vertex buffer objects
class VBO
{
protected:
	GLuint bufferId;

	VBO(const void* pData, int size);

	void bind();
	void unbind();

	friend VAO;
};

// Class for creating and managing index buffer objects
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

// Class for creating and managing vertex array objects
class VAO
{
public:
	// Structure that stores all data for a given vertex
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

	void enableAttribArrays(int data);

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