#pragma once
#include <unordered_map>
#include "IA.h"
#include "VertexBuffer.h"
#include "Technique.h"
#include "Transform.h"
#include "ConstantBuffer.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

	// technique has: Material, RenderState, Attachments (color, depth, etc)
	Technique* technique; 

	// translation buffers
	ConstantBuffer* txBuffer;
	// local copy of the translation
	Transform* transform;

	struct VertexBufferBind {
		size_t numElements, offset;
		VertexBuffer* buffer;
	};
	
	// array of buffers with locations (binding points in shaders)
	void addIAVertexBufferBinding(VertexBuffer* buffer, size_t offset, size_t size, unsigned int inputStream);
	void bindIAVertexBuffer(unsigned int location);
	std::unordered_map<unsigned int, VertexBufferBind> geometryBuffers;
};
