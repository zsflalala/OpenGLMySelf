#pragma once

namespace validation_utils
{
	struct SMesh;

	class CPrimitive
	{
	public:
		static void createCube(SMesh& voMesh);
		static void createSphere(SMesh& voMesh, float vRadius = 0.5f, unsigned int vLatitudeSegments = 16, unsigned int vLongitudeSegments = 32);
		static void createCone(SMesh& voMesh, float vRadius = 0.5f, float vHeight = 1.0f, unsigned int vSegments = 32);
	};
}
