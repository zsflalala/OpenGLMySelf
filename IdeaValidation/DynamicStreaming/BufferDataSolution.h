#pragma once

#include "DynamicStreamingSolution.h"

class CBufferDataSolution final : public CDynamicStreamingSolution
{
public:
	void initCopyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions) override
	{
		glBindBuffer(GL_ARRAY_BUFFER, vVBO);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(glm::vec3) * vPositions.size()), vPositions.data(), GL_DYNAMIC_DRAW);
	}

	void copyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions, size_t vStartIdx,
		size_t vEndIdx) override
	{
		glBufferSubData(
			GL_ARRAY_BUFFER,
			static_cast<GLintptr>(sizeof(glm::vec3) * vStartIdx),
			static_cast<GLsizeiptr>(sizeof(glm::vec3) * (vEndIdx - vStartIdx)),
			&vPositions[vStartIdx]
		);
	}
};

