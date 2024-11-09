#pragma once
#include <string>
#include <tiny_gltf.h>
#include <vector>

const int TINYGLTF_MODE_DEFAULT = -1;
const int TINYGLTF_MODE_POINT = 0;
const int TINYGLTF_MODE_TRIANGLE = 4;
const int TINYGLTF_COMPONETTYPE_UNSHORT = 5123;
const int TINYGLTF_COMPONETTYPE_UNINT = 5125;
const int TINYGLTF_COMPONETTYPE_FLOAT = 5126;

class CDragonGLTFModel
{
public:
	CDragonGLTFModel(const std::string& vFilename);
	
	bool initModel();
	const std::vector<float>         getVerticesVector()  { return m_Vertices; };
	const std::vector<unsigned int>  getIndicesVector()   { return m_Indices; };
	const std::vector<unsigned char> getImageVector()     { return m_ImageData; };
	const int						 getPerVectFloatNum() { return m_PerVertDataFloatNum; };
	const int						 getImageWidth()      { return m_ImageWidth; };
	const int						 getImageHeight()     { return m_ImageHeight; };

private:
	bool __loadGLTF(const std::string& vFilename);
	void __createImageData();
	void __createVerticeAndIndice();
	void __createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType);
	void __createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vOffset, const int vVecBytes);

	int				           m_PerVertDataFloatNum = 8;
	int						   m_ImageWidth			 = 0;
	int						   m_ImageHeight		 = 0;
	std::string				   m_FileName;
	tinygltf::Model			   m_ModelGLTF;
	std::vector<float>		   m_Vertices;
	std::vector<unsigned int>  m_Indices;
	std::vector<unsigned char> m_ImageData;
};

