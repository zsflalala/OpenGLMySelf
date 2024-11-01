#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class CDynamicStreamingSolution
{
public:
	CDynamicStreamingSolution() = default;
	virtual ~CDynamicStreamingSolution() = default;

	virtual void initCopyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions) = 0;
	virtual void copyDataToBuffer(GLuint vVBO, const std::vector<glm::vec3>& vPositions, size_t vStartIdx, size_t vEndIdx) = 0;
	virtual void shutdown() {}
};