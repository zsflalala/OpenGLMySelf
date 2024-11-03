#include "MultiDrawIndirectSolution.h"
#include "DrawCallProblem.h"
#include "ShaderProgram.h"
#include "types.h"

void CMultiDrawIndirectSolution::init(const CDrawCallProblem* vProblem)
{
	CDrawCallSolution::init(vProblem);
	__createAndFillGeometryBuffer(m_pProblem->getMeshes());
	CDrawCallSolution::_createAndFillAndBindShaderStorageBuffer(m_pProblem->getMeshes());
	__createAndFillDrawCmdBuffer(m_pProblem->getMeshes());

	m_pProgram = validation_utils::CShaderProgram::createShaderProgram("shaders/MultiDrawIndirect.vert", "shaders/MultiDrawIndirect.frag");
	m_pProgram->use();
	m_pProgram->setUniform("uViewProjection", m_pProblem->getViewProjMatrix());

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawCmdBuffer);
}

void CMultiDrawIndirectSolution::render()
{
	glMultiDrawElementsIndirect(
		GL_TRIANGLES,
		GL_UNSIGNED_INT,
		nullptr,   
		static_cast<GLsizei>(m_pProblem->getMeshes().size()),
		0
	);
}

void CMultiDrawIndirectSolution::onFameBufferResize(int vWidth, int vHeight)
{
	m_pProgram->use();
	m_pProgram->setUniform("uViewProjection", m_pProblem->getViewProjMatrix());
}

void CMultiDrawIndirectSolution::shutdown()
{
	m_pProgram.reset();
}

void CMultiDrawIndirectSolution::__createAndFillGeometryBuffer(const std::vector<validation_utils::SMesh>& vMeshes)
{
	std::vector<validation_utils::SVertex> AllVertices;
	std::vector<unsigned int> AllIndices;
	for (const auto& Mesh : vMeshes)
	{
		AllVertices.insert(AllVertices.end(), Mesh._Vertices.begin(), Mesh._Vertices.end());
		AllIndices.insert(AllIndices.end(), Mesh._Indices.begin(), Mesh._Indices.end());
	}

	GLuint VBO, EBO;
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(validation_utils::SVertex) * AllVertices.size()), AllVertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(validation_utils::SVertex), reinterpret_cast<GLvoid*>((offsetof(validation_utils::SVertex, _Position))));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(validation_utils::SVertex), reinterpret_cast<GLvoid*>((offsetof(validation_utils::SVertex, _Normal))));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(validation_utils::SVertex), reinterpret_cast<GLvoid*>((offsetof(validation_utils::SVertex, _UV))));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * AllIndices.size()), AllIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void CMultiDrawIndirectSolution::__createAndFillDrawCmdBuffer(const std::vector<validation_utils::SMesh>& vMeshes)
{
	std::vector<validation_utils::SDrawElementsIndirectCommand> Commands(vMeshes.size());
	size_t FirstIndex = 0;
	size_t BaseVertex = 0;
	for (size_t i = 0; i < Commands.size(); ++i)
	{
		Commands[i]._Count = static_cast<GLuint>(vMeshes[i]._Indices.size());
		Commands[i]._InstanceCount = 1;
		Commands[i]._FirstIndex = static_cast<GLuint>(FirstIndex);
		Commands[i]._BaseVertex = static_cast<GLint>(BaseVertex);
		Commands[i]._BaseInstance = 0; // TODO

		FirstIndex += vMeshes[i]._Indices.size();
		BaseVertex += vMeshes[i]._Vertices.size();
	}

	glCreateBuffers(1, &m_DrawCmdBuffer);
	glNamedBufferStorage(m_DrawCmdBuffer,
		static_cast<GLsizeiptr>(sizeof(validation_utils::SDrawElementsIndirectCommand) * Commands.size()),
		Commands.data(),
		GL_DYNAMIC_STORAGE_BIT
	);
}
