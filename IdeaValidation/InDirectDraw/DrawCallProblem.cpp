#include "DrawCallProblem.h"
#include <format>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "safe.h"
#include "types.h"
#include "utils.h"
#include "Primitive.h"
#include "DrawCallSolution.h"
#include "Timer.h"

CDrawCallProblem::CDrawCallProblem(int vSize, float vSpacing, const glm::vec3& vCamPos, bool vIsWireFrameMode)
	:m_pWindow(nullptr), m_pSolution(nullptr), m_Meshes{}, m_ViewProjMatrix{}, m_Size(vSize), m_Spacing(vSpacing), m_CamPos(vCamPos), m_IsWireFrameMode(vIsWireFrameMode)
{
}

void CDrawCallProblem::solve(CDrawCallSolution* vSolution, int vWidth, int vHeight)
{
	_ASSERTE(vSolution != nullptr);
	m_pSolution = vSolution;
	if (!__init(vWidth, vHeight)) return;
	std::cout << "Start Render\n";
	__render();
	__shutdown();
}

void CDrawCallProblem::updateViewProjMatrix()
{
	int Width, Height;
	glfwGetFramebufferSize(m_pWindow, &Width, &Height);
	const glm::mat4 ViewMatrix = glm::lookAt(
		m_CamPos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	const glm::mat4 ProjMatrix = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(Width) / static_cast<float>(Height),
		0.1f,
		10000.0f
	);
	m_ViewProjMatrix = ProjMatrix * ViewMatrix;
}

bool CDrawCallProblem::__init(int vWidth, int vHeight)
{
	_ASSERTE(m_pSolution != nullptr);
	m_pWindow = validation_utils::createWindow(vWidth, vHeight, "Draw Call Problem");
	if (m_pWindow == nullptr) return false;
	glfwSetWindowUserPointer(m_pWindow, this);
	glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* vWindow, int vNewWidth, int vNewHeight) {
		GL_SAFE_CALL(glViewport(0, 0, vNewWidth, vNewHeight));
		const auto pProblem = static_cast<CDrawCallProblem*>(glfwGetWindowUserPointer(vWindow));
		pProblem->updateViewProjMatrix();
		pProblem->getSolution()->onFameBufferResize(vNewWidth, vNewHeight);
		});
	__prepareMeshesData();
	updateViewProjMatrix();
	GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_SAFE_CALL(glViewport(0, 0, vWidth, vHeight));
	GL_SAFE_CALL(glClearColor(0.1f, 0.2f, 0.3f, 1.0f));
	GL_SAFE_CALL(glClearDepth(1.0f));
	GL_SAFE_CALL(glEnable(GL_DEPTH_TEST));
	GL_SAFE_CALL(glDepthMask(GL_TRUE));
	GL_SAFE_CALL(glPolygonMode(GL_FRONT_AND_BACK, m_IsWireFrameMode ? GL_LINE : GL_FILL));
	m_pSolution->init(this);
	return true;
}

void CDrawCallProblem::__shutdown()
{
	m_pSolution->shutdown();
	glfwDestroyWindow(m_pWindow);
	m_pWindow = nullptr;
	glfwTerminate();
}

void CDrawCallProblem::__render() const
{
	validation_utils::CTimer Timer;
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Timer.start();
		m_pSolution->render();
		Timer.getDeltaTime();
		glfwSwapBuffers(m_pWindow); // TODO: Why is larger size more time-consuming?
	}
}

void CDrawCallProblem::__prepareMeshesData()
{
	const int HalfSize = m_Size / 2;
	m_Meshes.clear();
	for (int Row = 0; Row < m_Size; ++Row)
	{
		for (int Col = 0; Col < m_Size; ++Col)
		{
			validation_utils::SMesh Mesh;
			if ((Row + Col) % 3 == 0)
				validation_utils::CPrimitive::createCube(Mesh);
			else if ((Row + Col) % 3 == 1)
				validation_utils::CPrimitive::createSphere(Mesh);
			else
				validation_utils::CPrimitive::createCone(Mesh);
			Mesh._ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
				m_Spacing * static_cast<float>(Col - HalfSize),
				m_Spacing * static_cast<float>(Row - HalfSize),
				0.0f
			));
			Mesh._Color = glm::vec4(
				validation_utils::random(0.0f, 1.0f),
				validation_utils::random(0.0f, 1.0f),
				validation_utils::random(0.0f, 1.0f),
				1.0f
			);
			m_Meshes.push_back(Mesh);
		}
	}
	std::cout << "Generated Meshes: " << m_Size * m_Size << '\n';
}
