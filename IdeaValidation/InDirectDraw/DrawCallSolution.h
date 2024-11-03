#pragma once

#include <glm/glm.hpp>
#include "types.h"
#include "safe.h"

class CDrawCallProblem;
class CDrawCallSolution
{
public:
	virtual ~CDrawCallSolution() = default;

	virtual void init(const CDrawCallProblem* vProblem) { m_pProblem = vProblem; }
	virtual void render() = 0;
	virtual void onFameBufferResize(int vWidth, int vHeight) = 0;
	virtual void shutdown() = 0;

protected:
	static void _createAndFillAndBindShaderStorageBuffer(const std::vector<validation_utils::SMesh>& vMeshes);
	const CDrawCallProblem* m_pProblem = nullptr;
};

inline void CDrawCallSolution::_createAndFillAndBindShaderStorageBuffer(const std::vector<validation_utils::SMesh>& vMeshes)
{
	std::vector<glm::mat4> ModelsBuffer(vMeshes.size());
	std::vector<glm::vec4> ColorsBuffer(vMeshes.size());
	for (size_t i = 0; i < vMeshes.size(); ++i)
	{
		ModelsBuffer[i] = vMeshes[i]._ModelMatrix;
		ColorsBuffer[i] = vMeshes[i]._Color;
	}

	GLuint ModelsSSBO;
	GL_SAFE_CALL(glCreateBuffers(1, &ModelsSSBO));
	GL_SAFE_CALL(glNamedBufferStorage(ModelsSSBO,
		static_cast<GLsizeiptr>(sizeof(glm::mat4) * ModelsBuffer.size()),
		ModelsBuffer.data(),
		GL_DYNAMIC_STORAGE_BIT
	));
	GL_SAFE_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ModelsSSBO));

	// DSA
	GLuint ColorsSSBO;
	GL_SAFE_CALL(glCreateBuffers(1, &ColorsSSBO));
	GL_SAFE_CALL(glNamedBufferStorage(ColorsSSBO,
		static_cast<GLsizeiptr>(sizeof(glm::vec4) * ColorsBuffer.size()),
		ColorsBuffer.data(),
		GL_DYNAMIC_STORAGE_BIT
	));
	GL_SAFE_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ColorsSSBO));
}
