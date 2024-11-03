#pragma once
#include <string>
#include <tiny_gltf.h>
#include <vector>

const int TINYGLTF_MODE_DEFAULT = -1;
const int TINYGLTF_MODE_POINT = 0;
const int TINYGLTF_MODE_TRIANGLE = 4;
const int TINYGLTF_COMPONETTYPE_UNSHORT = 5123;
const int TINYGLTF_COMPONETTYPE_UNINT = 5125;

class CDragonGLTFModel
{
public:
	CDragonGLTFModel(const std::string& vFilename);
	
	bool initModel();
	const std::vector<float>        getVerticesVector() { return m_Vertices; };
	const std::vector<unsigned int> getIndicesVector() { return m_Indices; };

private:
	bool __loadGLTF(const std::string& vFilename);
	void __createVerticeAndIndice();
	void __createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType);
	void __createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex);

	std::string				  m_FileName;
	tinygltf::Model			  m_ModelGLTF;
	std::vector<float>		  m_Vertices;
	std::vector<unsigned int> m_Indices;
};

