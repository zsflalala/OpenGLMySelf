#include "DragonGLTFModel.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

CDragonGLTFModel::CDragonGLTFModel(const std::string& vFilename)
{
    m_FileName = vFilename;
}

bool CDragonGLTFModel::initModel()
{
    bool LoadRes = __loadGLTF(m_FileName);
    if (!LoadRes) return LoadRes;
    __createVerticeAndIndice();
    return true;
}

bool CDragonGLTFModel::__loadGLTF(const std::string& vFilename)
{
    tinygltf::TinyGLTF Loader;
    std::string Err;
    std::string Warn;

    bool Res = Loader.LoadASCIIFromFile(&m_ModelGLTF, &Err, &Warn, vFilename);

    if (!Warn.empty())
    {
        std::cout << "WARN : " << Warn;
    }

    if (!Err.empty())
    {
        std::cout << "ERR : " << Err;
    }

    if (!Res)
    {
        std::cout << "Failed to load glTF : " << vFilename << std::endl;
    }
    else
    {
        std::cout << "Loaded glTF : " << vFilename << std::endl;
    }

    return Res;
}

void CDragonGLTFModel::__createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType)
{
    unsigned short TempUShortIndice;
    unsigned int   TempUIntIndice;
    const int UnShortByte = 2;
    const int UnIntByte = 4;
    if (vComponentType == TINYGLTF_COMPONETTYPE_UNSHORT)
    {
        for (size_t i = vBufferView.byteOffset; i < vBufferView.byteOffset + vBufferView.byteLength; i += UnShortByte)
        {
            std::memcpy(&TempUShortIndice, &vBuffer.data.at(i), sizeof(unsigned short));
            vIndices.push_back(TempUShortIndice);
        }
    }
    else if (vComponentType == TINYGLTF_COMPONETTYPE_UNINT)
    {
        for (size_t i = vBufferView.byteOffset; i < vBufferView.byteOffset + vBufferView.byteLength; i += UnIntByte)
        {
            std::memcpy(&TempUIntIndice, &vBuffer.data.at(i), sizeof(unsigned int));
            vIndices.push_back(TempUIntIndice);
        }
    }
}

void CDragonGLTFModel::__createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex)
{
    float TempVertice;
    const int FloatByte = 4;
    const int FloatNum = 3;
    for (auto i = vIndex; i < vIndex + FloatNum * FloatByte; i += FloatByte)
    {
        std::memcpy(&TempVertice, &vBuffer.data.at(i), sizeof(float));
        vVertices.push_back(TempVertice);
    }
}

void CDragonGLTFModel::__createVerticeAndIndice()
{
    for (auto& Node : m_ModelGLTF.nodes)
    {
        if (Node.mesh == -1) continue;
        const auto& Mesh = m_ModelGLTF.meshes[Node.mesh];
        std::string MeshName = Mesh.name;
        std::cout << "MeshName : " << MeshName << std::endl;

        for (auto& Primitive : Mesh.primitives)
        {
            m_Vertices.clear();
            if (Primitive.mode == TINYGLTF_MODE_POINT)
            {
                const tinygltf::Accessor& AccessorPos = m_ModelGLTF.accessors[Primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& BufferViewPos = m_ModelGLTF.bufferViews[AccessorPos.bufferView];
                const tinygltf::Buffer& BufferPos = m_ModelGLTF.buffers[BufferViewPos.buffer];
                const tinygltf::Accessor& AccessorColor = m_ModelGLTF.accessors[Primitive.attributes.at("COLOR_0")];
                const tinygltf::BufferView& BufferViewColor = m_ModelGLTF.bufferViews[AccessorColor.bufferView];
                const tinygltf::Buffer& BufferColor = m_ModelGLTF.buffers[BufferViewColor.buffer];
                glm::vec3 MinPos(AccessorPos.minValues[0], AccessorPos.minValues[1], AccessorPos.minValues[2]);
                glm::vec3 MaxPos(AccessorPos.maxValues[0], AccessorPos.maxValues[1], AccessorPos.maxValues[2]);

                const int Vec3Byte = 12;
                for (size_t i = BufferViewPos.byteOffset, k = BufferViewColor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewColor.byteOffset + BufferViewColor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    __createVertexBufferData(m_Vertices, BufferPos, (int)i);
                    __createVertexBufferData(m_Vertices, BufferColor, (int)k);
                }

                std::cout << "Vertices.size : " << m_Vertices.size() << std::endl;
                assert(m_Vertices.size() == m_ModelGLTF.accessors[Primitive.attributes.at("POSITION")].count * 3 * 2);
            }
            else if (Primitive.mode == TINYGLTF_MODE_TRIANGLE || Primitive.mode == TINYGLTF_MODE_DEFAULT)
            {
                m_Vertices.clear();
                m_Indices.clear();
                const tinygltf::BufferView& BufferViewIndice = m_ModelGLTF.bufferViews[m_ModelGLTF.accessors[Primitive.indices].bufferView];
                const tinygltf::Buffer& BufferIndice = m_ModelGLTF.buffers[BufferViewIndice.buffer];
                const int IndiceComponentType = m_ModelGLTF.accessors[Primitive.indices].componentType;

                __createIndiceBufferData(m_Indices, BufferViewIndice, BufferIndice, IndiceComponentType);
                std::cout << "indice.size : " << m_Indices.size() << std::endl;
                assert(m_Indices.size() == m_ModelGLTF.accessors[Primitive.indices].count);

                const tinygltf::BufferView& BufferViewPos = m_ModelGLTF.bufferViews[m_ModelGLTF.accessors[Primitive.attributes.at("POSITION")].bufferView];
                const tinygltf::Buffer& BufferPos = m_ModelGLTF.buffers[BufferViewPos.buffer];
                const tinygltf::BufferView& BufferViewNor = m_ModelGLTF.bufferViews[m_ModelGLTF.accessors[Primitive.attributes.at("NORMAL")].bufferView];
                const tinygltf::Buffer& BufferNor = m_ModelGLTF.buffers[BufferViewNor.buffer];

                assert(BufferViewPos.byteLength == BufferViewNor.byteLength);

                const int Vec3Byte = 12;
                for (std::size_t i = BufferViewPos.byteOffset, k = BufferViewNor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewNor.byteOffset + BufferViewNor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    __createVertexBufferData(m_Vertices, BufferPos, (int)i);
                    __createVertexBufferData(m_Vertices, BufferNor, (int)k);
                }
                std::cout << "Vertices.size : " << m_Vertices.size() << std::endl;
                assert(m_Vertices.size() == m_ModelGLTF.accessors[Primitive.attributes.at("POSITION")].count * 6);
            }
        }
    }
    return;
}