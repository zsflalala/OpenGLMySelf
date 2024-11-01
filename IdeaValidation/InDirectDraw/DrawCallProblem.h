#pragma once
#include <vector>
#include "types.h"

struct GLFWwindow;
class CDrawCallSolution;
class CDrawCallProblem
{
public:
	CDrawCallProblem(int vSize, float vSpacing, const glm::vec3& vCamPos, bool vIsWireFrameMode);
	[[nodiscard]] const auto& getSolution() const { return m_pSolution; }
	[[nodiscard]] const auto& getMeshes() const { return m_Meshes; }
	[[nodiscard]] const auto& getViewProjMatrix() const { return m_ViewProjMatrix; }
	void solve(CDrawCallSolution* vSolution, int vWidth, int vHeight);
	void updateViewProjMatrix();

private:
	bool __init(int vWidth, int vHeight);
	void __shutdown();
	void __render() const;
	void __prepareMeshesData();

	GLFWwindow* m_pWindow;
	CDrawCallSolution* m_pSolution;
	std::vector<validation_utils::SMesh> m_Meshes;
	glm::mat4 m_ViewProjMatrix;

	int m_Size;
	float m_Spacing;
	glm::vec3 m_CamPos;
	bool m_IsWireFrameMode;
};

