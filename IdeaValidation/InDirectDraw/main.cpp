#include "DrawCallProblem.h"
#include "MultiDrawIndirectSolution.h"
#include "BasicDrawSolution.h"

int main()
{
	CDrawCallProblem Problem(100, 1.5f, glm::vec3(0.0f, 0.0f, 150.0f), false);
	CMultiDrawIndirectSolution Solution;
	//CBasicDrawSolution Solution;
	Problem.solve(&Solution, 800, 600);
	return 0;
}