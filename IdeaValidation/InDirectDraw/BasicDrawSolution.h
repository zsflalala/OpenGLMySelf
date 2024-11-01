#pragma once

#include <memory>
#include <vector>
#include <glad/glad.h>
#include "DrawCallSolution.h"

namespace validation_utils
{
	class CShaderProgram;
}

class CBasicDrawSolution final : public CDrawCallSolution
{
public:
	CBasicDrawSolution() = default;

	void init(const CDrawCallProblem* vProblem) override;
	void render() override;
	void onFameBufferResize(int vWidth, int vHeight) override;
	void shutdown() override;

private:
	void __createAndFillGeometryBuffer(const std::vector<validation_utils::SMesh>& vMeshes);

	std::shared_ptr<validation_utils::CShaderProgram> m_pProgram;
	std::vector<GLuint> m_VAOs;
};

