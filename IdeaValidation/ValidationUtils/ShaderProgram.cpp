#include "pch.h"
#include "ShaderProgram.h"
#include "safe.h"

using namespace validation_utils;

CShaderProgram::CShaderProgram()
{
	m_ProgramID = glCreateProgram();
}

CShaderProgram::~CShaderProgram()
{
	__deleteShaderIDs();
	GL_SAFE_CALL(glDeleteProgram(m_ProgramID));
}

bool CShaderProgram::attachShader(const EShaderType& vShaderType, const std::string& vShaderPath)
{
	std::string ShaderCode;
	if (!__dumpShaderCodeFromFile(vShaderPath, vShaderType, ShaderCode))
	{
		return false;
	}

	GLuint ShaderID;
	if (!__compileShader(ShaderCode, vShaderType, ShaderID))
	{
		return false;
	}

	GL_SAFE_CALL(glAttachShader(m_ProgramID, ShaderID));
	m_ShaderIDs.insert(ShaderID);
	return true;
}

bool CShaderProgram::linkProgram()
{
	GL_SAFE_CALL(glLinkProgram(m_ProgramID));
	if (!__checkLinkError(m_ProgramID))
	{
		__deleteShaderIDs();
		GL_SAFE_CALL(glDeleteProgram(m_ProgramID));
		return false;
	}
	__deleteShaderIDs();
	return true;
}

void CShaderProgram::use() const
{
	GL_SAFE_CALL(glUseProgram(m_ProgramID));
}

void CShaderProgram::setUniform(const std::string& vName, int vValue)
{
	GL_SAFE_CALL(glUniform1iv(__getOrCreateUniformID(vName), 1, &vValue));
}

void CShaderProgram::setUniform(const std::string& vName, float vValue)
{
	GL_SAFE_CALL(glUniform1fv(__getOrCreateUniformID(vName), 1, &vValue));
}

void CShaderProgram::setUniform(const std::string& vName, const glm::vec2& vValue)
{
	GL_SAFE_CALL(glUniform2fv(__getOrCreateUniformID(vName), 1, &vValue[0]));
}

void CShaderProgram::setUniform(const std::string& vName, const glm::vec3& vValue)
{
	GL_SAFE_CALL(glUniform3fv(__getOrCreateUniformID(vName), 1, &vValue[0]));
}

void CShaderProgram::setUniform(const std::string& vName, const std::vector<glm::vec3>& vValues)
{
	GL_SAFE_CALL(glUniform3fv(__getOrCreateUniformID(vName), static_cast<GLsizei>(vValues.size()), &vValues[0].x));
}

void CShaderProgram::setUniform(const std::string& vName, const glm::vec4& vValue)
{
	GL_SAFE_CALL(glUniform4fv(__getOrCreateUniformID(vName), 1, &vValue[0]));
}

void CShaderProgram::setUniform(const std::string& vName, const glm::mat4& vMat)
{
	GL_SAFE_CALL(glUniformMatrix4fv(__getOrCreateUniformID(vName), 1, GL_FALSE, &vMat[0][0]));
}

void CShaderProgram::setUniform(const std::string& vName, const glm::mat3& vMat)
{
	GL_SAFE_CALL(glUniformMatrix3fv(__getOrCreateUniformID(vName), 1, GL_FALSE, &vMat[0][0]));
}

std::shared_ptr<CShaderProgram> CShaderProgram::createShaderProgram(const std::string& vVertPath, const std::string& vFragPath, const std::string& vGeomPath)
{
	const auto& pProgram = std::make_shared<CShaderProgram>();
	pProgram->attachShader(EShaderType::VERTEX, vVertPath);
	pProgram->attachShader(EShaderType::FRAGMENT, vFragPath);
	if (!vGeomPath.empty()) pProgram->attachShader(EShaderType::GEOMETRY, vGeomPath);
	pProgram->linkProgram();
	return pProgram;
}

bool CShaderProgram::__dumpShaderCodeFromFile(const std::string& vShaderPath, EShaderType vShaderType, std::string& voShaderCode)
{
	std::ifstream ShaderFile;
	ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		ShaderFile.open(vShaderPath);
		std::stringstream ShaderStream;
		ShaderStream << ShaderFile.rdbuf();
		ShaderFile.close();
		voShaderCode = ShaderStream.str();
		return true;
	}
	catch (std::ifstream::failure& Error)
	{
		std::cerr << "{} shader file READ failed:" << __getShaderTypeName(vShaderType) << vShaderPath << Error.what();
		return false;
	}
}

bool CShaderProgram::__compileShader(const std::string& vShaderCode, EShaderType vShaderType, GLuint& voShaderID)
{
	voShaderID = glCreateShader(static_cast<GLenum>(vShaderType));
	const char* ShaderCodeCStr = vShaderCode.c_str();
	GL_SAFE_CALL(glShaderSource(voShaderID, 1, &ShaderCodeCStr, nullptr));
	GL_SAFE_CALL(glCompileShader(voShaderID));
	if (!__checkCompileError(voShaderID, vShaderType))
	{
		GL_SAFE_CALL(glDeleteShader(voShaderID));
		return false;
	}
	return true;
}

bool CShaderProgram::__checkCompileError(GLuint vID, EShaderType vShaderType)
{
	GLint IsSuccess;
	GL_SAFE_CALL(glGetShaderiv(vID, GL_COMPILE_STATUS, &IsSuccess));
	if (!IsSuccess)
	{
		GLchar InfoLog[1024];
		GL_SAFE_CALL(glGetShaderInfoLog(vID, 1024, nullptr, InfoLog));
		std::cerr << "shader COMPILE failed:" << __getShaderTypeName(vShaderType) << '\n' << InfoLog;
		return false;
	}
	return true;
}

bool CShaderProgram::__checkLinkError(GLuint vID)
{
	GLint IsSuccess;
	GL_SAFE_CALL(glGetProgramiv(vID, GL_LINK_STATUS, &IsSuccess));
	if (!IsSuccess)
	{
		GLchar InfoLog[1024];
		GL_SAFE_CALL(glGetProgramInfoLog(vID, 1024, nullptr, InfoLog));
		std::cerr << "shader LINK failed: \n" << InfoLog;
		return false;
	}
	return true;
}

std::string CShaderProgram::__getShaderTypeName(EShaderType vShaderType)
{
	if (vShaderType == EShaderType::VERTEX) return "VERTEX";
	if (vShaderType == EShaderType::FRAGMENT) return "FRAGMENT";
	if (vShaderType == EShaderType::GEOMETRY) return "GEOMETRY";
	return "UNDEFINED";
}

void CShaderProgram::__deleteShaderIDs()
{
	for (const auto& ShaderID : m_ShaderIDs)
	{
		GL_SAFE_CALL(glDeleteShader(ShaderID));
	}
	m_ShaderIDs.clear();
}

GLint CShaderProgram::__getOrCreateUniformID(const std::string& vName)
{
	if (m_UniformIDMap.contains(vName)) return m_UniformIDMap[vName];
	const auto ShaderID = glGetUniformLocation(m_ProgramID, vName.c_str());
	if (ShaderID == -1)
	{
		std::cerr << "uniform {} does not exist or is not set correctly " << vName;
	}
	m_UniformIDMap[vName] = ShaderID;
	return ShaderID;
}
