#pragma once
#include<string>
#include <glm/glm.hpp>

class CShader
{
public:
	CShader(const char* ComputeShaderPath);
	CShader(const char* VertexPath, const char* FragmentPath);
	const char* VertexSource;
	const char* FragmentSource;
	const char* ComputeSource;
	unsigned int ID; //ShaderProgram's ID;
	void Use();
	void SetBool(const std::string &Name, bool Value)const;
	void SetInt(const std::string &Name, int Value)const;
	void SetFloat(const std::string &Name, float Value)const;
	void SetVec2(const std::string &Name, glm::vec2 Value)const;
	void SetVec3(const std::string &Name, glm::vec3 Value)const;
	void SetVec3(const std::string& Name, float x, float y, float z)const;
	void SetMatrix4(const std::string& Name, int Count, int WhetherTranspose, glm::mat4 Value)const;

	~CShader();
private:
	void CheckCompileErrors(unsigned int ID, std::string type);
};

