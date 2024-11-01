#include "BasicDrawSolution.h"
#include "DrawCallProblem.h"
#include "ShaderProgram.h"

void CBasicDrawSolution::init(const CDrawCallProblem* vProblem)
{
	CDrawCallSolution::init(vProblem);
	m_pProgram = validation_utils::CShaderProgram::createShaderProgram("shaders/BasicDraw.vert", "shaders/BasicDraw.frag");
	m_pProgram->use();
	m_pProgram->setUniform("uViewProjection", m_pProblem->getViewProjMatrix());

	__createAndFillGeometryBuffer(m_pProblem->getMeshes());
	CDrawCallSolution::_createAndFillAndBindShaderStorageBuffer(m_pProblem->getMeshes());
}

void CBasicDrawSolution::render()
{
	const auto& Meshes = m_pProblem->getMeshes();
	for (size_t i = 0; i < m_VAOs.size(); ++i)
	{
		glBindVertexArray(m_VAOs[i]);
		m_pProgram->setUniform("uDrawID", static_cast<int>(i));
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(Meshes[i]._Indices.size()), GL_UNSIGNED_INT, nullptr);
	}
}

void CBasicDrawSolution::onFameBufferResize(int vWidth, int vHeight)
{
	m_pProgram->use();
	m_pProgram->setUniform("uViewProjection", m_pProblem->getViewProjMatrix());
}

void CBasicDrawSolution::shutdown()
{
	m_pProgram.reset();
	glDeleteVertexArrays(static_cast<GLsizei>(m_VAOs.size()), m_VAOs.data());
}

void CBasicDrawSolution::__createAndFillGeometryBuffer(const std::vector<validation_utils::SMesh>& vMeshes)
{
	m_VAOs.resize(vMeshes.size());
	std::vector<GLuint> VBOs(vMeshes.size());
	std::vector<GLuint> EBOs(vMeshes.size());
	glGenVertexArrays(static_cast<GLsizei>(m_VAOs.size()), m_VAOs.data());
	glGenBuffers(static_cast<GLsizei>(VBOs.size()), VBOs.data());
	glGenBuffers(static_cast<GLsizei>(EBOs.size()), EBOs.data());
	
	for (size_t i = 0; i < vMeshes.size(); ++i)
	{
		glBindVertexArray(m_VAOs[i]);

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(validation_utils::SVertex) * vMeshes[i]._Vertices.size()), vMeshes[i]._Vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(validation_utils::SVertex), reinterpret_cast<GLvoid*>((offsetof(validation_utils::SVertex, _Position))));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(validation_utils::SVertex), reinterpret_cast<GLvoid*>((offsetof(validation_utils::SVertex, _Normal))));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(validation_utils::SVertex), reinterpret_cast<GLvoid*>((offsetof(validation_utils::SVertex, _UV))));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * vMeshes[i]._Indices.size()), vMeshes[i]._Indices.data(), GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}
