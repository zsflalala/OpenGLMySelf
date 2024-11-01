#include "pch.h"
#include "Primitive.h"
#include "types.h"

using namespace validation_utils;

void CPrimitive::createCube(SMesh& voMesh)
{
	constexpr glm::vec3 Positions[] = {
		{-0.5f, -0.5f, -0.5f}, // 0
		{ 0.5f, -0.5f, -0.5f}, // 1
		{ 0.5f,  0.5f, -0.5f}, // 2
		{-0.5f,  0.5f, -0.5f}, // 3
		{-0.5f, -0.5f,  0.5f}, // 4
		{ 0.5f, -0.5f,  0.5f}, // 5
		{ 0.5f,  0.5f,  0.5f}, // 6
		{-0.5f,  0.5f,  0.5f}  // 7
	};
	constexpr glm::vec3 Normals[] = {
		{ 0.0f,  0.0f, -1.0f}, // Front
		{ 0.0f,  0.0f,  1.0f}, // Back
		{-1.0f,  0.0f,  0.0f}, // Left
		{ 1.0f,  0.0f,  0.0f}, // Right
		{ 0.0f, -1.0f,  0.0f}, // Bottom
		{ 0.0f,  1.0f,  0.0f}  // Top
	};
	constexpr glm::vec2 UVs[] = {
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f}
	};
	voMesh._Vertices = {
		// Front face
		{Positions[0], Normals[0], UVs[0]},
		{Positions[1], Normals[0], UVs[1]},
		{Positions[2], Normals[0], UVs[2]},
		{Positions[3], Normals[0], UVs[3]},

		// Back face
		{Positions[5], Normals[1], UVs[0]},
		{Positions[4], Normals[1], UVs[1]},
		{Positions[7], Normals[1], UVs[2]},
		{Positions[6], Normals[1], UVs[3]},

		// Left face
		{Positions[4], Normals[2], UVs[0]},
		{Positions[0], Normals[2], UVs[1]},
		{Positions[3], Normals[2], UVs[2]},
		{Positions[7], Normals[2], UVs[3]},

		// Right face
		{Positions[1], Normals[3], UVs[0]},
		{Positions[5], Normals[3], UVs[1]},
		{Positions[6], Normals[3], UVs[2]},
		{Positions[2], Normals[3], UVs[3]},

		// Bottom face
		{Positions[4], Normals[4], UVs[0]},
		{Positions[5], Normals[4], UVs[1]},
		{Positions[1], Normals[4], UVs[2]},
		{Positions[0], Normals[4], UVs[3]},

		// Top face
		{Positions[3], Normals[5], UVs[0]},
		{Positions[2], Normals[5], UVs[1]},
		{Positions[6], Normals[5], UVs[2]},
		{Positions[7], Normals[5], UVs[3]}
	};
	voMesh._Indices = {
		// Front face
		0, 1, 2,
		2, 3, 0,

		// Back face
		4, 5, 6,
		6, 7, 4,

		// Left face
		8, 9, 10,
		10, 11, 8,

		// Right face
		12, 13, 14,
		14, 15, 12,

		// Bottom face
		16, 17, 18,
		18, 19, 16,

		// Top face
		20, 21, 22,
		22, 23, 20
	};
}

void CPrimitive::createSphere(SMesh& voMesh, float vRadius, unsigned int vLatitudeSegments, unsigned int vLongitudeSegments)
{
	voMesh._Vertices.clear();
	voMesh._Indices.clear();

	for (unsigned int Lat = 0; Lat <= vLatitudeSegments; ++Lat)
	{
		const float Theta = static_cast<float>(Lat) * glm::pi<float>() / static_cast<float>(vLatitudeSegments);
		const float SinTheta = sin(Theta);
		const float CosTheta = cos(Theta);

		for (unsigned int Lon = 0; Lon <= vLongitudeSegments; ++Lon)
		{
			const float Phi = static_cast<float>(Lon) * 2.0f * glm::pi<float>() / static_cast<float>(vLongitudeSegments);
			const float SinPhi = sin(Phi);
			const float CosPhi = cos(Phi);

			glm::vec3 Position(
				vRadius * CosPhi * SinTheta,
				vRadius * CosTheta,
				vRadius * SinPhi * SinTheta
			);

			const glm::vec3 Normal = glm::normalize(Position);
			const glm::vec2 UV(
				static_cast<float>(Lon) / static_cast<float>(vLongitudeSegments),
				static_cast<float>(Lat) / static_cast<float>(vLatitudeSegments)
			);

			voMesh._Vertices.push_back({ Position, Normal, UV });
		}
	}

	for (unsigned int Lat = 0; Lat < vLatitudeSegments; ++Lat)
	{
		for (unsigned int Lon = 0; Lon < vLongitudeSegments; ++Lon)
		{
			unsigned int First = (Lat * (vLongitudeSegments + 1)) + Lon;
			unsigned int Second = First + vLongitudeSegments + 1;

			voMesh._Indices.push_back(First);
			voMesh._Indices.push_back(Second);
			voMesh._Indices.push_back(First + 1);

			voMesh._Indices.push_back(Second);
			voMesh._Indices.push_back(Second + 1);
			voMesh._Indices.push_back(First + 1);
		}
	}
}

void CPrimitive::createCone(SMesh& voMesh, float vRadius, float vHeight, unsigned int vSegments)
{
	voMesh._Vertices.clear();
	voMesh._Indices.clear();

	float HalfHeight = vHeight / 2.0f;

	SVertex CenterVertex;
	CenterVertex._Position = glm::vec3(0.0f, -HalfHeight, 0.0f);
	CenterVertex._Normal = glm::vec3(0.0f, -1.0f, 0.0f);
	CenterVertex._UV = glm::vec2(0.5f, 0.5f);
	voMesh._Vertices.push_back(CenterVertex);

	SVertex TopVertex;
	TopVertex._Position = glm::vec3(0.0f, HalfHeight, 0.0f);
	TopVertex._Normal = glm::normalize(glm::vec3(0.0f, vHeight, 0.0f));
	TopVertex._UV = glm::vec2(0.5f, 1.0f);
	voMesh._Vertices.push_back(TopVertex);

	for (unsigned int i = 0; i <= vSegments; ++i)
	{
		float Angle = static_cast<float>(i) * 2.0f * glm::pi<float>() / static_cast<float>(vSegments);
		float X = vRadius * cos(Angle);
		float Z = vRadius * sin(Angle);

		auto Position = glm::vec3(X, -HalfHeight, Z);
		auto Normal = glm::normalize(glm::vec3(X, HalfHeight, Z));
		auto UV = glm::vec2((X / vRadius + 1.0f) * 0.5f, (Z / vRadius + 1.0f) * 0.5f);

		voMesh._Vertices.push_back({ Position, Normal, UV });
	}

	for (unsigned int i = 1; i <= vSegments; ++i)
	{
		voMesh._Indices.push_back(0);
		voMesh._Indices.push_back(i + 1);
		voMesh._Indices.push_back(i);
	}

	for (unsigned int i = 1; i <= vSegments; ++i)
	{
		voMesh._Indices.push_back(1);
		voMesh._Indices.push_back(i);
		voMesh._Indices.push_back(i + 1);
	}
}
