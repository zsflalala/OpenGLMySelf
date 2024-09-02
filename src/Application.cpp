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
bool __loadGLTF(const std::string& vFilename, tinygltf::Model& vModelGLTF);
void __createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType);
void __createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex);
void __createVerticeAndIndice(tinygltf::Model& vGLTFModel, std::vector<float>& vioVertices, std::vector<unsigned int>& vioIndices); 

const int TINYGLTF_MODE_DEFAULT = -1;
const int TINYGLTF_MODE_POINT = 0;
const int TINYGLTF_MODE_TRIANGLE = 4;
const int TINYGLTF_COMPONETTYPE_UNSHORT = 5123;
const int TINYGLTF_COMPONETTYPE_UNINT = 5125;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    tinygltf::Model GLTFModel;
    __loadGLTF("./models/dragon.gltf", GLTFModel);
    std::vector<float> Vertices;
    std::vector<unsigned int> Indices;
    __createVerticeAndIndice(GLTFModel, Vertices, Indices);
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
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // - Create depth texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
    Shader DragonShader("./shaders/shadowMapping.vs", "./shaders/shadowMapping.fs");
    Shader DepthShader("./shaders/shadowMappingDepth.vs", "./shaders/shadowMappingDepth.fs");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        GLfloat near_plane = 1.0f, far_plane = 7.5f;
        lightPos.x = sin(glfwGetTime()) * 2.0f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        DepthShader.use();
        DepthShader.setMat4("lightSpaceMatrix", lightProjection);
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        DepthShader.setMat4("model", model);
        
        // Draw the scene through the light
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glBindVertexArray(DragonVAO);
        glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DragonShader.use();
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        DragonShader.setMat4("projection", projection);
        DragonShader.setMat4("view", view);
        DragonShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        DragonShader.setVec3("lightPos", lightPos);
        DragonShader.setVec3("viewPos", camera.Position);
        DragonShader.setBool("shadows",true);
        DragonShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        // Draw the normal model
        glBindVertexArray(DragonVAO);
        glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &DragonVBO);
    glDeleteFramebuffers(1, &depthMapFBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

bool __loadGLTF(const std::string& vFilename, tinygltf::Model& vModelGLTF)
{
    tinygltf::TinyGLTF Loader;
    std::string Err;
    std::string Warn;

    bool res = Loader.LoadASCIIFromFile(&vModelGLTF, &Err, &Warn, vFilename);

    if (!Warn.empty()) {
        std::cout << "WARN: {}" << Warn;
    }

    if (!Err.empty()) {
        std::cout << "ERR: {}" << Err;
    }

    if (!res) {
        std::cout << "Failed to load glTF: {}" << vFilename;
    }
    else {
        std::cout << "Loaded glTF: {}" << vFilename;
    }

    return res;
}

void __createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType)
{
    unsigned short tempUShortIndice;
    unsigned int   tempUIntIndice;
    const int UnShortByte = 2;
    const int UnIntByte = 4;
    if (vComponentType == TINYGLTF_COMPONETTYPE_UNSHORT)
    {
        for (size_t i = vBufferView.byteOffset; i < vBufferView.byteOffset + vBufferView.byteLength; i += UnShortByte) {
            std::memcpy(&tempUShortIndice, &vBuffer.data.at(i), sizeof(unsigned short));
            vIndices.push_back(tempUShortIndice);
        }
    }
    else if (vComponentType == TINYGLTF_COMPONETTYPE_UNINT)
    {
        for (size_t i = vBufferView.byteOffset; i < vBufferView.byteOffset + vBufferView.byteLength; i += UnIntByte) {
            std::memcpy(&tempUIntIndice, &vBuffer.data.at(i), sizeof(unsigned int));
            vIndices.push_back(tempUIntIndice);
        }
    }
}

void __createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex) {
    float tempVertice;
    const int FloatByte = 4;
    const int FloatNum = 3;
    for (auto i = vIndex; i < vIndex + FloatNum * FloatByte; i += FloatByte)
    {
        std::memcpy(&tempVertice, &vBuffer.data.at(i), sizeof(float));
        vVertices.push_back(tempVertice);
    }
}

void __createVerticeAndIndice(tinygltf::Model& vGLTFModel, std::vector<float>& vioVertices, std::vector<unsigned int>& vioIndices)
{
    for (auto& node : vGLTFModel.nodes)
    {
        if (node.mesh == -1) continue;
        const auto& Mesh = vGLTFModel.meshes[node.mesh];
        std::string MeshName = Mesh.name;
        std::cout << "MeshName : {}" << MeshName << std::endl;

        for (auto& primitive : Mesh.primitives)
        {
            vioVertices.clear();
            if (primitive.mode == TINYGLTF_MODE_POINT)
            {
                const tinygltf::Accessor& AccessorPos = vGLTFModel.accessors[primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& BufferViewPos = vGLTFModel.bufferViews[AccessorPos.bufferView];
                const tinygltf::Buffer& BufferPos = vGLTFModel.buffers[BufferViewPos.buffer];
                const tinygltf::Accessor& AccessorColor = vGLTFModel.accessors[primitive.attributes.at("COLOR_0")];
                const tinygltf::BufferView& BufferViewColor = vGLTFModel.bufferViews[AccessorColor.bufferView];
                const tinygltf::Buffer& BufferColor = vGLTFModel.buffers[BufferViewColor.buffer];
                glm::vec3 MinPos(AccessorPos.minValues[0], AccessorPos.minValues[1], AccessorPos.minValues[2]);
                glm::vec3 MaxPos(AccessorPos.maxValues[0], AccessorPos.maxValues[1], AccessorPos.maxValues[2]);

                const int Vec3Byte = 12;
                for (size_t i = BufferViewPos.byteOffset, k = BufferViewColor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewColor.byteOffset + BufferViewColor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    __createVertexBufferData(vioVertices, BufferPos, (int)i);
                    __createVertexBufferData(vioVertices, BufferColor, (int)k);
                }

                std::cout << "Vertices.size : {}" << vioVertices.size() << std::endl;
                assert(vioVertices.size() == vGLTFModel.accessors[primitive.attributes.at("POSITION")].count * 3 * 2);
            }
            else if (primitive.mode == TINYGLTF_MODE_TRIANGLE || primitive.mode == TINYGLTF_MODE_DEFAULT)
            {
                vioVertices.clear();
                vioIndices.clear();
                const tinygltf::BufferView& BufferViewIndice = vGLTFModel.bufferViews[vGLTFModel.accessors[primitive.indices].bufferView];
                const tinygltf::Buffer& BufferIndice = vGLTFModel.buffers[BufferViewIndice.buffer];
                const int IndiceComponentType = vGLTFModel.accessors[primitive.indices].componentType;

                __createIndiceBufferData(vioIndices, BufferViewIndice, BufferIndice, IndiceComponentType);
                std::cout << "indice.size : {}" << vioIndices.size();
                assert(vioIndices.size() == vGLTFModel.accessors[primitive.indices].count);

                const tinygltf::BufferView& BufferViewPos = vGLTFModel.bufferViews[vGLTFModel.accessors[primitive.attributes.at("POSITION")].bufferView];
                const tinygltf::Buffer& BufferPos = vGLTFModel.buffers[BufferViewPos.buffer];
                const tinygltf::BufferView& BufferViewNor = vGLTFModel.bufferViews[vGLTFModel.accessors[primitive.attributes.at("NORMAL")].bufferView];
                const tinygltf::Buffer& BufferNor = vGLTFModel.buffers[BufferViewNor.buffer];

                assert(BufferViewPos.byteLength == BufferViewNor.byteLength);

                const int Vec3Byte = 12;
                for (std::size_t i = BufferViewPos.byteOffset, k = BufferViewNor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewNor.byteOffset + BufferViewNor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    __createVertexBufferData(vioVertices, BufferPos, (int)i);
                    __createVertexBufferData(vioVertices, BufferNor, (int)k);
                }
                std::cout << "Vertices.size : {}" << vioVertices.size() << std::endl;
                assert(vioVertices.size() == vGLTFModel.accessors[primitive.attributes.at("POSITION")].count * 6);
            }
        }
    }
    return;
}