#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <tiny_gltf.h>
#include "shader.h"
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
bool loadGLTF(const std::string& vFilename, tinygltf::Model& vModelGLTF);
void createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType);
void createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex);
void createVerticeAndIndice(tinygltf::Model& vGLTFModel, std::vector<float>& vioVertices, std::vector<unsigned int>& vioIndices); 

const int TINYGLTF_MODE_DEFAULT = -1;
const int TINYGLTF_MODE_POINT = 0;
const int TINYGLTF_MODE_TRIANGLE = 4;
const int TINYGLTF_COMPONETTYPE_UNSHORT = 5123;
const int TINYGLTF_COMPONETTYPE_UNINT = 5125;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

CCamera Camera(glm::vec3(0.0f, 0.0f, 30.0f));

bool FirstMouse = true;
float Yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float Pitch = 0.0f;
float LastX = 800.0f / 2.0;
float LastY = 600.0 / 2.0;
float Fov = 45.0f;

// timing
float DeltaTime = 0.0f;	// time between current frame and last frame
float LastFrame = 0.0f;


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* pWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (pWindow == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(pWindow);
    glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(pWindow, mouse_callback);
    glfwSetScrollCallback(pWindow, scroll_callback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    tinygltf::Model GLTFModel;
    loadGLTF("./models/dragon.gltf", GLTFModel);
    std::vector<float> Vertices;
    std::vector<unsigned int> Indices;
    createVerticeAndIndice(GLTFModel, Vertices, Indices);
    unsigned int DragonVAO, DragonVBO, DragonEBO;
    glGenVertexArrays(1, &DragonVAO);
    glGenBuffers(1, &DragonVBO);
    glGenBuffers(1, &DragonEBO);

    glBindVertexArray(DragonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, DragonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DragonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), Indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    GLuint DepthMap;
    glCreateTextures(GL_TEXTURE_2D, 1, &DepthMap);
    glTextureStorage2D(DepthMap, 1, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT);
    glTextureParameteri(DepthMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(DepthMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameteri(DepthMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(DepthMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLfloat BorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(DepthMap, GL_TEXTURE_BORDER_COLOR, BorderColor);

    GLuint DepthMapFBO;
    glGenFramebuffers(1, &DepthMapFBO); 
    glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glm::vec4 LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 LightPos = glm::vec3(1.2f, 1.0f, 2.0f);
    CShader DragonShader("./shaders/shadowMapping.vs", "./shaders/shadowMapping.fs");
    CShader DepthShader("./shaders/shadowMappingDepth.vs", "./shaders/shadowMappingDepth.fs");

    while (!glfwWindowShouldClose(pWindow))
    {
        float CurrentFrame = static_cast<float>(glfwGetTime());
        DeltaTime = CurrentFrame - LastFrame;
        LastFrame = CurrentFrame;
        processInput(pWindow);

        GLfloat NearPlane = 1.0f, FarPlane = 7.5f;
        LightPos.x = static_cast<float>(sin(glfwGetTime()) * 2.0f);
        glm::mat4 LightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, NearPlane, FarPlane);
        glm::mat4 LightView = glm::lookAt(LightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 LightSpaceMatrix = LightProjection * LightView;
        DepthShader.use();
        DepthShader.setMat4("lightSpaceMatrix", LightProjection);
        glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        DepthShader.setMat4("model", Model);
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glBindVertexArray(DragonVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(Indices.size()), GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DragonShader.use();
        glm::mat4 Projection = glm::perspective(Camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 View = Camera.GetViewMatrix();
        DragonShader.setMat4("projection", Projection);
        DragonShader.setMat4("view", View);
        DragonShader.setMat4("lightSpaceMatrix", LightSpaceMatrix);
        DragonShader.setVec3("lightPos", LightPos);
        DragonShader.setVec3("viewPos", Camera.Position);
        DragonShader.setBool("shadows",true);
        DragonShader.setMat4("model", Model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, DepthMap);

        glBindVertexArray(DragonVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(Indices.size()), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &DragonVBO);
    glDeleteFramebuffers(1, &DepthMapFBO);
    glfwDestroyWindow(pWindow);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float CameraSpeed = static_cast<float>(2.5 * DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Camera.ProcessKeyboard(FORWARD, DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Camera.ProcessKeyboard(BACKWARD, DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Camera.ProcessKeyboard(LEFT, DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Camera.ProcessKeyboard(RIGHT, DeltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float PosX = static_cast<float>(xposIn);
    float PosY = static_cast<float>(yposIn);

    if (FirstMouse)
    {
        LastX = PosX;
        LastY = PosY;
        FirstMouse = false;
    }

    float OffsetX = PosX - LastX;
    float OffsetY = LastY - PosY; // reversed since y-coordinates go from bottom to top
    LastX = PosX;
    LastY = PosY;

    Camera.ProcessMouseMovement(OffsetX, OffsetY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int Width, Height, NrComponents;
    unsigned char* pData = stbi_load(path, &Width, &Height, &NrComponents, 0);
    if (pData)
    {
        GLenum format = GL_RGB;
        if (NrComponents == 1)
            format = GL_RED;
        else if (NrComponents == 3)
            format = GL_RGB;
        else if (NrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, format, GL_UNSIGNED_BYTE, pData);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(pData);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(pData);
    }

    return textureID;
}

bool loadGLTF(const std::string& vFilename, tinygltf::Model& vModelGLTF)
{
    tinygltf::TinyGLTF Loader;
    std::string Err;
    std::string Warn;

    bool res = Loader.LoadASCIIFromFile(&vModelGLTF, &Err, &Warn, vFilename);

    if (!Warn.empty()) {
        std::cout << "WARN : " << Warn;
    }

    if (!Err.empty()) {
        std::cout << "ERR : " << Err;
    }

    if (!res) {
        std::cout << "Failed to load glTF : " << vFilename << std::endl;
    }
    else {
        std::cout << "Loaded glTF : " << vFilename << std::endl;
    }

    return res;
}

void createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType)
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

void createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex) 
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

void createVerticeAndIndice(tinygltf::Model& vGLTFModel, std::vector<float>& vioVertices, std::vector<unsigned int>& vioIndices)
{
    for (auto& Node : vGLTFModel.nodes)
    {
        if (Node.mesh == -1) continue;
        const auto& Mesh = vGLTFModel.meshes[Node.mesh];
        std::string MeshName = Mesh.name;
        std::cout << "MeshName : " << MeshName << std::endl;

        for (auto& Primitive : Mesh.primitives)
        {
            vioVertices.clear();
            if (Primitive.mode == TINYGLTF_MODE_POINT)
            {
                const tinygltf::Accessor& AccessorPos = vGLTFModel.accessors[Primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& BufferViewPos = vGLTFModel.bufferViews[AccessorPos.bufferView];
                const tinygltf::Buffer& BufferPos = vGLTFModel.buffers[BufferViewPos.buffer];
                const tinygltf::Accessor& AccessorColor = vGLTFModel.accessors[Primitive.attributes.at("COLOR_0")];
                const tinygltf::BufferView& BufferViewColor = vGLTFModel.bufferViews[AccessorColor.bufferView];
                const tinygltf::Buffer& BufferColor = vGLTFModel.buffers[BufferViewColor.buffer];
                glm::vec3 MinPos(AccessorPos.minValues[0], AccessorPos.minValues[1], AccessorPos.minValues[2]);
                glm::vec3 MaxPos(AccessorPos.maxValues[0], AccessorPos.maxValues[1], AccessorPos.maxValues[2]);

                const int Vec3Byte = 12;
                for (size_t i = BufferViewPos.byteOffset, k = BufferViewColor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewColor.byteOffset + BufferViewColor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    createVertexBufferData(vioVertices, BufferPos, (int)i);
                    createVertexBufferData(vioVertices, BufferColor, (int)k);
                }

                std::cout << "Vertices.size : " << vioVertices.size() << std::endl;
                assert(vioVertices.size() == vGLTFModel.accessors[Primitive.attributes.at("POSITION")].count * 3 * 2);
            }
            else if (Primitive.mode == TINYGLTF_MODE_TRIANGLE || Primitive.mode == TINYGLTF_MODE_DEFAULT)
            {
                vioVertices.clear();
                vioIndices.clear();
                const tinygltf::BufferView& BufferViewIndice = vGLTFModel.bufferViews[vGLTFModel.accessors[Primitive.indices].bufferView];
                const tinygltf::Buffer& BufferIndice = vGLTFModel.buffers[BufferViewIndice.buffer];
                const int IndiceComponentType = vGLTFModel.accessors[Primitive.indices].componentType;

                createIndiceBufferData(vioIndices, BufferViewIndice, BufferIndice, IndiceComponentType);
                std::cout << "indice.size : " << vioIndices.size() << std::endl;
                assert(vioIndices.size() == vGLTFModel.accessors[Primitive.indices].count);

                const tinygltf::BufferView& BufferViewPos = vGLTFModel.bufferViews[vGLTFModel.accessors[Primitive.attributes.at("POSITION")].bufferView];
                const tinygltf::Buffer& BufferPos = vGLTFModel.buffers[BufferViewPos.buffer];
                const tinygltf::BufferView& BufferViewNor = vGLTFModel.bufferViews[vGLTFModel.accessors[Primitive.attributes.at("NORMAL")].bufferView];
                const tinygltf::Buffer& BufferNor = vGLTFModel.buffers[BufferViewNor.buffer];

                assert(BufferViewPos.byteLength == BufferViewNor.byteLength);

                const int Vec3Byte = 12;
                for (std::size_t i = BufferViewPos.byteOffset, k = BufferViewNor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewNor.byteOffset + BufferViewNor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    createVertexBufferData(vioVertices, BufferPos, (int)i);
                    createVertexBufferData(vioVertices, BufferNor, (int)k);
                }
                std::cout << "Vertices.size : " << vioVertices.size() << std::endl;
                assert(vioVertices.size() == vGLTFModel.accessors[Primitive.attributes.at("POSITION")].count * 6);
            }
        }
    }
    return;
}