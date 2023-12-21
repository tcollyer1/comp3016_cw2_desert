#include <glad/glad.h>
#include <GLFW/glfw3.h>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\h\Buffers.h"

using namespace glm;

VAO::VAO()
{
	// Set index of the VAO.
	glGenVertexArrays(1, &vaoId);

	verticesBuffer = NULL;
	indicesBuffer = NULL;
}

VAO::~VAO()
{
	unbind();
}

// Gets the size of each coordinate (x2, x3 etc.) for a given type of buffer.
int VAO::getCoordSize(BufferType type)
{
	int sz = 0;

	switch (type)
	{
	case COLOURS:
		sz = 4;
		break;
	case VERTICES:
	case NORMALS:
		sz = 3;
		break;
	case TEXTURES:
		sz = 2;
		break;
	default:
		break;
	}

	return (sz);
}

// Creates a new vertex or index buffer object.
void VAO::addBuffer(const void* pData, int size, BufferType type)
{
	switch (type)
	{
	case VERTICES:
		verticesBuffer = new VBO(pData, size);
		break;
	case INDICES:
		indicesBuffer = new IBO(pData, size);
		break;
	default:
		// Invalid
		break;
	}	
}

// Enables requested vertex arrays from the following: BUF_VERTICES | BUF_NORMALS | BUF_TEXTURES | BUF_COLOURS
void VAO::enableAttribArrays(int data)
{
	if (data & BUF_VERTICES)
	{
		// Vertices
		glVertexAttribPointer(VERTICES, getCoordSize(VERTICES), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, vertices));
		glEnableVertexAttribArray(VERTICES);
	}
	if (data & BUF_NORMALS)
	{
		// Normals
		glVertexAttribPointer(NORMALS, getCoordSize(NORMALS), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normals));
		glEnableVertexAttribArray(NORMALS);
	}
	if (data & BUF_TEXTURES)
	{
		// Textures
		glVertexAttribPointer(TEXTURES, getCoordSize(TEXTURES), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, textures));
		glEnableVertexAttribArray(TEXTURES);
	}
	if (data & BUF_COLOURS)
	{
		// Colours
		glVertexAttribPointer(COLOURS, getCoordSize(COLOURS), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, colours));
		glEnableVertexAttribArray(COLOURS);
	}
}

void VAO::bind()
{
	// Bind VAO to current context.
	glBindVertexArray(vaoId);
}

void VAO::unbind()
{
	// Unbind VAO
	glBindVertexArray(0);
}

VBO::VBO(const void* pData, int size)
{
	// Set index for VBO
	glGenBuffers(1, &bufferId);

	// Bind VBO to an array buffer type for drawing.
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);

	// Allocate buffer memory for the vertices.
	glBufferData(GL_ARRAY_BUFFER, size, pData, GL_STATIC_DRAW);
}

VBO::~VBO()
{
	unbind();
}

void VBO::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
}

void VBO::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IBO::IBO(const void* pData, int size)
{
	glGenBuffers(1, &bufferId);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);

	// Allocate buffer memory for the indices.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, pData, GL_STATIC_DRAW);
}

IBO::~IBO()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}