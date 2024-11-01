#include "pch.h"
#include "utils.h"

GLFWwindow* validation_utils::createWindow(int vWidth, int vHeight, const std::string& vTitle, int vMajorVersion, int vMinorVersion)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, vMajorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, vMinorVersion);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* pWindow = glfwCreateWindow(vWidth, vHeight, vTitle.c_str(), nullptr, nullptr);
	if (pWindow == nullptr)
	{
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(pWindow);
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD\n";
		glfwDestroyWindow(pWindow);
		glfwTerminate();
		return nullptr;
	}
	return pWindow;
}

int validation_utils::random(int vMin, int vMax)
{
	return vMin + rand() % (vMax - vMin + 1);
}

float validation_utils::random(float vMin, float vMax)
{
	return vMin + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (vMax - vMin);
}
