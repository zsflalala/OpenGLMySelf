#pragma once

#include <memory>
#include <vector>
#include "DrawCallSolution.h"
#include <glad/glad.h>

namespace validation_utils
{
	class CShaderProgram;
}

class CMultiDrawIndirectSolution final : public CDrawCallSolution
{
public:
	CMultiDrawIndirectSolution() = default;

	void init(const CDrawCallProblem* vProblem) override;
	void render() override;
	void onFameBufferResize(int vWidth, int vHeight) override;
	void shutdown() override;

private:
	void __createAndFillGeometryBuffer(const std::vector<validation_utils::SMesh>& vMeshes);
	void __createAndFillDrawCmdBuffer(const std::vector<validation_utils::SMesh>& vMeshes);

	GLuint m_VAO;
	GLuint m_DrawCmdBuffer;
	std::shared_ptr<validation_utils::CShaderProgram> m_pProgram;
};

