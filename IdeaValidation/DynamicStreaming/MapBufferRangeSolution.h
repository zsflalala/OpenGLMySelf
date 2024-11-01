#pragma once
#include "DynamicStreamingSolution.h"

class CMapBufferRangeSolution final : public CDynamicStreamingSolution
{
public:
	void initCopyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions) override
	{
		m_VBO = vVBO;
		glBindBuffer(GL_ARRAY_BUFFER, vVBO);
		const GLsizeiptr BufferSize = static_cast<GLsizeiptr>(sizeof(glm::vec3) * vPositions.size());
		constexpr GLbitfield Flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glBufferStorage(GL_ARRAY_BUFFER, BufferSize, nullptr, Flags);
		m_pData = static_cast<glm::vec3*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, Flags));
		memcpy(m_pData, vPositions.data(), BufferSize);
	}

	void copyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions, size_t vStartIdx,
		size_t vEndIdx) override
	{
		memcpy(
			m_pData + vStartIdx,
			&vPositions[vStartIdx].x,
			sizeof(glm::vec3) * (vEndIdx - vStartIdx)
		);
	}

	void shutdown() override
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

private:
	GLuint m_VBO{};
	glm::vec3* m_pData = nullptr;
};

