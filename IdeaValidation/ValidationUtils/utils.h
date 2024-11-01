#pragma once

#include <string>

struct GLFWwindow;

namespace validation_utils
{
	GLFWwindow* createWindow(int vWidth, int vHeight, const std::string& vTitle, int vMajorVersion = 4, int vMinorVersion = 6);

	int random(int vMin, int vMax);

	float random(float vMin, float vMax);
}
