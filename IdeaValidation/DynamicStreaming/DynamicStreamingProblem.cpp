#include "DynamicStreamingProblem.h"
#include <format>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "utils.h"
#include "safe.h"
#include "DynamicStreamingSolution.h"
#include "ShaderProgram.h"
#include "Timer.h"

CDynamicStreamingProblem::CDynamicStreamingProblem(size_t vNumOfPoints, size_t vUpdateBatchSize)
	:m_pWindow(nullptr), m_pSolution(nullptr),m_VAO{}, m_VBO{}, m_pProgram(nullptr), m_Positions{}, m_Velocities{}, m_NumOfPoints(vNumOfPoints), m_UpdateBatchSize(vUpdateBatchSize)
{
}

void CDynamicStreamingProblem::solve(CDynamicStreamingSolution* vSolution, int vWidth, int vHeight)
{
	_ASSERTE(vSolution != nullptr);
	m_pSolution = vSolution;
	if (!__init(vWidth, vHeight)) return;
	__initPointsData();
	__render();
	__shutdown();
}

bool CDynamicStreamingProblem::__init(int vWidth, int vHeight)
{
	m_pWindow = validation_utils::createWindow(vWidth, vHeight, "Dynamic Streaming");
	if (m_pWindow == nullptr) return false;
	glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* vWindow, int vNewWidth, int vNewHeight)
	{
		glViewport(0, 0, vNewWidth, vNewHeight);
	});
	
	m_pProgram = validation_utils::CShaderProgram::createShaderProgram("shaders/particle.vert", "shaders/particle.frag");
	if (m_pProgram == nullptr) return false;

	GL_SAFE_CALL(glGenVertexArrays(1, &m_VAO));
	GL_SAFE_CALL(glGenBuffers(1, &m_VBO));
	GL_SAFE_CALL(glBindVertexArray(m_VAO));
	GL_SAFE_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
	GL_SAFE_CALL(glEnableVertexAttribArray(0));
	GL_SAFE_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr)));
	GL_SAFE_CALL(glBindVertexArray(0));

	GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_SAFE_CALL(glViewport(0, 0, vWidth, vHeight));
	GL_SAFE_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
	GL_SAFE_CALL(glClearDepth(1.0f));
	GL_SAFE_CALL(glEnable(GL_DEPTH_TEST));
	GL_SAFE_CALL(glDepthMask(GL_TRUE));

	return true;
}

void CDynamicStreamingProblem::__render()
{
	m_pProgram->use();
	m_pProgram->setUniform("uColor", glm::vec3(1.0f, 0.0f, 0.0f));
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glPointSize(3.0f);
	validation_utils::CTimer UpdateDataCostTimer;
	float LastTime = 0.0f;
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();
		const float CurrTime = static_cast<float>(glfwGetTime());
		const float DeltaTime = CurrTime - LastTime;
		LastTime = CurrTime;
		glfwSetWindowTitle(m_pWindow, std::format("Dynamic Streaming FPS: {}", static_cast<int>(1.0f / DeltaTime)).c_str());
		UpdateDataCostTimer.start();
		__updatePointsData(DeltaTime);
		UpdateDataCostTimer.getDeltaTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_Positions.size()));
		glfwSwapBuffers(m_pWindow);
	}
	std::cout << "Avg Update Data Cost: " << UpdateDataCostTimer.getAverageDeltaTime() << "ms\n";
}

void CDynamicStreamingProblem::__shutdown()
{
	m_pSolution->shutdown();
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	m_pProgram.reset();
	glfwDestroyWindow(m_pWindow);
	m_pWindow = nullptr;
	glfwTerminate();
}

void CDynamicStreamingProblem::__initPointsData()
{
	m_Positions.resize(m_NumOfPoints);
	m_Velocities.resize(m_NumOfPoints);
	for (auto& Point : m_Positions)
	{
		Point = glm::vec3(
			validation_utils::random(-1.0f, 1.0f),
			validation_utils::random(-1.0f, 1.0f),
			0.0f
		);
	}
	for (auto& Velocity : m_Velocities)
	{
		Velocity = glm::vec3(
			validation_utils::random(-1.0f, 1.0f),
			validation_utils::random(-1.0f, 1.0f),
			0.0f
		);
	}
	m_pSolution->initCopyDataToBuffer(m_VBO, m_Positions);
}

void CDynamicStreamingProblem::__updatePointsData(float vDeltaTime)
{
	for (size_t i = 0; i < m_Positions.size(); ++i)
	{
		constexpr float Speed = 0.1f;
		m_Positions[i] += m_Velocities[i] * Speed * vDeltaTime;
		if (m_Positions[i].x < -1.0f || m_Positions[i].x > 1.0f)
			m_Velocities[i].x = -m_Velocities[i].x;
		if (m_Positions[i].y < -1.0f || m_Positions[i].y > 1.0f)
			m_Velocities[i].y = -m_Velocities[i].y;
	}
	size_t i = 0;
	for (; i < m_Positions.size() / m_UpdateBatchSize; ++i)
	{
		m_pSolution->copyDataToBuffer(m_VBO, m_Positions, i * m_UpdateBatchSize, (i + 1) * m_UpdateBatchSize);
		//std::cout << "update " << i * m_UpdateBatchSize << ", " << (i + 1) * m_UpdateBatchSize << '\n';
	}
	if (const size_t RestSize = m_Positions.size() % m_UpdateBatchSize; RestSize != 0)
	{
		m_pSolution->copyDataToBuffer(m_VBO, m_Positions, i * m_UpdateBatchSize, i * m_UpdateBatchSize + RestSize);
		//std::cout << "update " << i * m_UpdateBatchSize << ", " << i * m_UpdateBatchSize + RestSize << '\n';
	}
}
