#include <chrono>
#include <cstring>
#include <random>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.h"

constexpr int PARTICLES_COUNT = 8192;

struct SParticle
{
	glm::vec2 _Position;
	glm::vec2 _Velocity;
	glm::vec4 _Color;
};

SParticle g_Particles[PARTICLES_COUNT];
GLvoid* g_MappedPtr = nullptr;

GLuint g_UBO;
GLuint g_ParticlesSSBO;

GLuint g_VAO;
GLuint g_VBO;

CShader* g_ComputeShader = nullptr;
CShader* g_ParticleShader = nullptr;

float g_LastFrameTime = 0.0f;
float g_LastTime = 0.0f;

GLFWwindow* createWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* pWindow = glfwCreateWindow(1024, 1024, "Particles Demo", nullptr, nullptr);
	if (pWindow == nullptr)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(pWindow);
	glfwSetFramebufferSizeCallback(pWindow, [](GLFWwindow*, int vWidth, int vHeight)
		{
			glViewport(0, 0, vWidth, vHeight);
		});
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize GLAD\n";
		return nullptr;
	}
	return pWindow;
}

void generateParticles()
{
	std::default_random_engine DefaultRndEngine(static_cast<unsigned>(time(nullptr)));
	std::uniform_real_distribution RndDist(0.0f, 1.0f);

	for (auto& [_Position, _Velocity, _Color] : g_Particles)
	{
		const float r = 0.25f * sqrt(RndDist(DefaultRndEngine));
		const float theta = RndDist(DefaultRndEngine) * 2.0f * 3.14159265358979323846f;
		const float x = r * cos(theta) * 800 / 600;
		const float y = r * sin(theta);
		_Position = glm::vec2(x, y);
		_Velocity = normalize(glm::vec2(x, y)) * 0.00025f;
		_Color = glm::vec4(RndDist(DefaultRndEngine), RndDist(DefaultRndEngine), RndDist(DefaultRndEngine), 1.0f);
	}
}

void generateBuffers() {
	GLint BindIndex = 0;
	const GLuint ubIndex = glGetUniformBlockIndex(g_ComputeShader->ID, "ParameterUBO");
	glUniformBlockBinding(g_ComputeShader->ID, ubIndex, BindIndex);

	GLint bufferSize = 0;
	glGetActiveUniformBlockiv(g_ComputeShader->ID, ubIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);

	glGenBuffers(1, &g_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, g_UBO);
	glBufferData(GL_UNIFORM_BUFFER, bufferSize, nullptr, GL_DYNAMIC_READ);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_UBO);

	BindIndex = 1;
	glGenBuffers(1, &g_ParticlesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_ParticlesSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(g_Particles), &g_Particles, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_ParticlesSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_ParticlesSSBO);
	g_MappedPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(g_MappedPtr, &g_Particles, sizeof(g_Particles));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glGenVertexArrays(1, &g_VAO);
	glBindVertexArray(g_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_ParticlesSSBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

int main()
{
	GLFWwindow* pWindow = createWindow();
	g_ComputeShader = new CShader("shaders/particle.comp");
	g_ParticleShader = new CShader("shaders/particle.vert", "shaders/particle.frag");

	generateParticles();
	generateBuffers();

	while (!glfwWindowShouldClose(pWindow))
	{

		//Clear Screen
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_ComputeShader->Use();
		glBindBuffer(GL_UNIFORM_BUFFER, g_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &g_LastFrameTime);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_ParticlesSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_ParticlesSSBO);

		glDispatchCompute(PARTICLES_COUNT / 256 + (PARTICLES_COUNT % 256 > 0 ? 1 : 0), 1, 1);

		//这部分可以从GPU获取数据
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ParticlesSSBO);
		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);//隔断作用，为了让数据修改完成
		//g_MappedPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY); //获取着色器buffer块内存地址
		//memcpy(&Particles, g_MappedPtr, sizeof(Particles));//拷贝buffer块数据到结构体
		//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);		

		g_ParticleShader->Use();
		glBindVertexArray(g_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, g_ParticlesSSBO);
		glPointSize(3.0f);
		glDrawArrays(GL_POINTS, 0, PARTICLES_COUNT);

		const double CurrentTime = glfwGetTime();
		g_LastFrameTime = (CurrentTime - g_LastTime) * 1000.0;
		g_LastTime = CurrentTime;

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}

	return 0;
}
