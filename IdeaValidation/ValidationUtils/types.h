#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace validation_utils
{
	struct SVertex
	{
		glm::vec3 _Position;
		glm::vec3 _Normal;
		glm::vec2 _UV;
	};

	struct SMesh
	{
		std::vector<SVertex> _Vertices;
		std::vector<unsigned int> _Indices;
		glm::mat4 _ModelMatrix;
		glm::vec4 _Color;
	};

	struct SDrawElementsIndirectCommand
	{
		GLuint  _Count;
		GLuint  _InstanceCount;
		GLuint  _FirstIndex;
		GLint  _BaseVertex;
		GLuint  _BaseInstance;
	};
	
}
