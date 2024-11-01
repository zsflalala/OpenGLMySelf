#pragma once

#include <memory>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "utils.h"

namespace validation_utils
{
	class CShaderProgram;
}

struct GLFWwindow;
class CDynamicStreamingSolution;
class CDynamicStreamingProblem
{
public:
	CDynamicStreamingProblem(size_t vNumOfPoints, size_t vUpdateBatchSize);

	[[nodiscard]] const auto& getSolution() const { return m_pSolution; }
	void solve(CDynamicStreamingSolution* vSolution, int vWidth, int vHeight);

private:
	bool __init(int vWidth, int vHeight);
	void __render();
	void __shutdown();
	void __initPointsData();
	void __updatePointsData(float vDeltaTime);

	GLFWwindow* m_pWindow;
	CDynamicStreamingSolution* m_pSolution;
	GLuint m_VAO, m_VBO;
	std::shared_ptr<validation_utils::CShaderProgram> m_pProgram;
	std::vector<glm::vec3> m_Positions;
	std::vector<glm::vec3> m_Velocities;
	size_t m_NumOfPoints;
	size_t m_UpdateBatchSize;
};

