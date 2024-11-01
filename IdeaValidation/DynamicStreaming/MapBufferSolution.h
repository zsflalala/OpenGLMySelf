#pragma once

#include <iostream>
#include "DynamicStreamingSolution.h"

class CMapBufferSolution final : public CDynamicStreamingSolution
{
public:
	void initCopyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions) override
	{
		glBindBuffer(GL_ARRAY_BUFFER, vVBO);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(glm::vec3) * vPositions.size()), nullptr, GL_DYNAMIC_DRAW);
		void* pData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(pData, vPositions.data(), sizeof(glm::vec3) * vPositions.size());
		const bool Status = glUnmapBuffer(GL_ARRAY_BUFFER);
		//std::cout << "Map Buffer Init Status: " << (Status ? "Success" : "Fail") << "\n";
	}

	void copyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions, size_t vStartIdx,
		size_t vEndIdx) override
	{
		const auto pData = static_cast<glm::vec3*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		memcpy(
			pData + vStartIdx,
			&vPositions[vStartIdx].x,
			sizeof(glm::vec3) * (vEndIdx - vStartIdx)
		);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
};
