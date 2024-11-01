#include "DynamicStreamingProblem.h"
#include "BufferDataSolution.h"
#include "MapBufferSolution.h"
#include "MapBufferRangeSolution.h"

int main()
{
	CDynamicStreamingProblem Problem(10000, 1);
	//CBufferDataSolution Solution;
	//CMapBufferSolution Solution;
	CMapBufferRangeSolution Solution;
	Problem.solve(&Solution, 800, 600);
	return 0;
}