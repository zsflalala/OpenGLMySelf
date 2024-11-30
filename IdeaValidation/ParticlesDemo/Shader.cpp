#include "Shader.h"
#include<fstream>
#include<iostream>
#include<sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.inl>

CShader::CShader(const char* ComputeShaderPath) 
{
	std::ifstream ComputeFile;
	std::stringstream ComputeSStream;
	std::string ComputeString;

	ComputeFile.open(ComputeShaderPath);
	ComputeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		if (!ComputeFile.is_open()) {
			throw std::exception("Open file error");
		}

		ComputeSStream << ComputeFile.rdbuf();

		ComputeString = ComputeSStream.str();

		ComputeSource = ComputeString.c_str();

		GLuint ComputeShader;

		ComputeShader = glCreateShader(GL_COMPUTE_SHADER);

		glShaderSource(ComputeShader, 1, &ComputeSource, NULL);
		glCompileShader(ComputeShader);
		CheckCompileErrors(ComputeShader, "COMPUTE");


		ID = glCreateProgram();
		glAttachShader(ID, ComputeShader);
		glLinkProgram(ID);
		CheckCompileErrors(ID, "PROGRAM");
		glDeleteShader(ComputeShader);
	}
	catch (const std::exception& ex)
	{
		printf(ex.what());
	}
}

CShader::CShader(const char *VertexPath, const char *FragmentPath) 
{
	std::ifstream VertexFile;
	std::ifstream FragmentFile;
	std::stringstream VertexSStream;
	std::stringstream FragmentSStream;
	std::string VertexString;
	std::string FragmentString;

	VertexFile.open(VertexPath);
	FragmentFile.open(FragmentPath);
	VertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	VertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		if (!VertexFile.is_open() || !FragmentFile.is_open()) {
			throw std::exception("Open file error");
		}

		VertexSStream << VertexFile.rdbuf();
		FragmentSStream << FragmentFile.rdbuf();

		VertexString = VertexSStream.str();
		FragmentString = FragmentSStream.str();

		VertexSource = VertexString.c_str();
		FragmentSource = FragmentString.c_str();

		unsigned int VertexShader, FragmentShader;

		VertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(VertexShader, 1, &VertexSource, NULL);
		glCompileShader(VertexShader);
		CheckCompileErrors(VertexShader, "VERTEX");

		FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(FragmentShader, 1, &FragmentSource, NULL);
		glCompileShader(FragmentShader);
		CheckCompileErrors(FragmentShader, "FRAGMENT");

		ID = glCreateProgram();
		glAttachShader(ID, VertexShader);
		glAttachShader(ID, FragmentShader);
		glLinkProgram(ID);
		CheckCompileErrors(ID, "PROGRAM");
		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);
	}
	catch (const std::exception& ex)
	{
		printf(ex.what());
	}
}
void CShader::Use() 
{
	glUseProgram(ID);
}
void CShader::SetBool(const std::string& Name, bool Value) const
{
	glUniform1i(glGetUniformLocation(ID, Name.c_str()), (int)Value);
}
void CShader::SetInt(const std::string& Name, int Value) const
{
	glUniform1i(glGetUniformLocation(ID, Name.c_str()), Value);
}
void CShader::SetFloat(const std::string& Name, float Value) const
{
	glUniform1f(glGetUniformLocation(ID, Name.c_str()), Value);
}
void CShader::SetVec2(const std::string& Name, glm::vec2 Value) const
{
	glUniform2f(glGetUniformLocation(ID, Name.c_str()), Value.x, Value.y);
}
void CShader::SetVec3(const std::string& Name, glm::vec3 Value) const
{
	glUniform3f(glGetUniformLocation(ID, Name.c_str()), Value.x, Value.y, Value.z);
}
void CShader::SetVec3(const std::string& Name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, Name.c_str()), x, y, z);
}
void CShader::SetMatrix4(const std::string& Name, int Count, int WhetherTranspose, glm::mat4 Value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, Name.c_str()), Count, WhetherTranspose, glm::value_ptr(Value));
}

void CShader::CheckCompileErrors(unsigned int ID, std::string Type) 
{
	int Success;
	char InfoLog[512];
	if (Type != "PROGRAM") {
		glGetShaderiv(ID, GL_COMPILE_STATUS, &Success);
		if (!Success) {
			glGetShaderInfoLog(ID, 512, NULL, InfoLog);
			std::cout << "CShader compile error: " << InfoLog << std::endl;
		}
	}
	else {
		glGetProgramiv(ID, GL_LINK_STATUS, &Success);
		if (!Success) {
			glGetProgramInfoLog(ID, 512, NULL, InfoLog);
			std::cout << "Program linking error: " << InfoLog << std::endl;
		}
	}
}

CShader::~CShader() 
{
	glDeleteProgram(ID);
}