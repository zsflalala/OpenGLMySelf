/*
  reference : https://ktstephano.github.io/rendering/opengl/mdi
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "DragonGLTFModel.h"
#include "camera.h"
#include "shader.h"

struct SDrawArraysIndirectCommand {
    GLuint _Count;
    GLuint _InstanceCount;
    GLuint _FirstIndex;
    GLuint _BaseVertex;
    GLuint _BaseInstance;
};

CCamera Camera(glm::vec3(0.0f, 0.0f, 30.0f));
float DeltaTime = 0.0f;
float LastFrame = 0.0f;
bool FirstMouse = true;
float Yaw = -90.0f;
float Pitch = 0.0f;
float LastX = 800.0f / 2.0;
float LastY = 600.0 / 2.0;
float Fov = 45.0f;

void framebufferSizeCallback(GLFWwindow* vWindow, int vWidth, int vHeight)
{
    glViewport(0, 0, vWidth, vHeight);
}

void processInput(GLFWwindow* vWindow)
{
    if (glfwGetKey(vWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(vWindow, true);

    float CameraSpeed = static_cast<float>(2.5 * DeltaTime);
    if (glfwGetKey(vWindow, GLFW_KEY_W) == GLFW_PRESS)
        Camera.ProcessKeyboard(FORWARD, DeltaTime);
    if (glfwGetKey(vWindow, GLFW_KEY_S) == GLFW_PRESS)
        Camera.ProcessKeyboard(BACKWARD, DeltaTime);
    if (glfwGetKey(vWindow, GLFW_KEY_A) == GLFW_PRESS)
        Camera.ProcessKeyboard(LEFT, DeltaTime);
    if (glfwGetKey(vWindow, GLFW_KEY_D) == GLFW_PRESS)
        Camera.ProcessKeyboard(RIGHT, DeltaTime);
}

void mouseCallback(GLFWwindow* vWindow, double vPosX, double vPosY)
{
    float PosX = static_cast<float>(vPosX);
    float PosY = static_cast<float>(vPosY);

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

void scrollCallback(GLFWwindow* vWindow, double vOffsetX, double vOffsetY)
{
    Camera.ProcessMouseScroll(static_cast<float>(vOffsetY));
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* pWindow = glfwCreateWindow(800, 600, "OpenGL IndirectDraw Demo", NULL, NULL);
    if (!pWindow)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(pWindow);
    glfwSetCursorPosCallback(pWindow, mouseCallback);
    glfwSetScrollCallback(pWindow, scrollCallback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    int WindowWidth, WindowHeight;
    glfwGetFramebufferSize(pWindow, &WindowWidth, &WindowHeight);
    glViewport(0, 0, WindowWidth, WindowHeight);
    glfwSetFramebufferSizeCallback(pWindow, framebufferSizeCallback);

    CDragonGLTFModel DragonModel("./models/dragon.gltf");
    if (!DragonModel.initModel()) return -1;
    std::vector<float> DragonVertices = DragonModel.getVerticesVector();
    std::vector<unsigned int> DragonIndices = DragonModel.getIndicesVector();

    // 指定三角形的顶点数据
    std::vector<float> TriangleVertices = {
         0.5f, -0.5f, 20.0f,   0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 20.0f,   0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 20.0f,   0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, 20.0f,   0.0f, 1.0f, 1.0f,
        -0.5f,  0.7f, 20.0f,   0.0f, 1.0f, 1.0f,
         0.58f, 0.3f, 20.0f,   0.0f, 1.0f, 1.0f
    };

    std::vector<unsigned int> TriangleIndices =
    {
        0, 1, 2,
        0, 1, 2
    };

    std::vector<float> TotalVertieces;
    std::vector<unsigned int> TotalIndices;

    TotalVertieces.insert(TotalVertieces.end(), TriangleVertices.begin(), TriangleVertices.end());
    TotalVertieces.insert(TotalVertieces.end(), DragonVertices.begin(), DragonVertices.end());
    TotalIndices.insert(TotalIndices.end(), TriangleIndices.begin(), TriangleIndices.end());
    TotalIndices.insert(TotalIndices.end(), DragonIndices.begin(), DragonIndices.end());

    unsigned int TotalVAO, TotalVBO, TotalEBO;
    glGenVertexArrays(1, &TotalVAO);
    glGenBuffers(1, &TotalVBO);
    glGenBuffers(1, &TotalEBO);
    glBindVertexArray(TotalVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TotalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * TotalVertieces.size(), TotalVertieces.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TotalEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * TotalIndices.size(), TotalIndices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    // 创建indirect draw命令
    const int DrawNum = 3;
    SDrawArraysIndirectCommand DrawCommand[DrawNum];
    std::cout << sizeof(SDrawArraysIndirectCommand) * DrawNum << " " << sizeof(DrawCommand) << std::endl;
    DrawCommand[0]._Count = 3;
    DrawCommand[0]._InstanceCount = 1;
    DrawCommand[0]._FirstIndex = 0;
    DrawCommand[0]._BaseVertex = 0;
    DrawCommand[0]._BaseInstance = 0;
    DrawCommand[1]._Count = 3;
    DrawCommand[1]._InstanceCount = 1;
    DrawCommand[1]._FirstIndex = 3;
    DrawCommand[1]._BaseVertex = 3;
    DrawCommand[1]._BaseInstance = 0;
    DrawCommand[2]._Count = DragonIndices.size();
    DrawCommand[2]._InstanceCount = 1;
    DrawCommand[2]._FirstIndex = TriangleIndices.size();
    DrawCommand[2]._BaseVertex = TriangleVertices.size() / 6;
    DrawCommand[2]._BaseInstance = 0;

    const int SCR_WIDTH  = 800;
    const int SCR_HEIGHT = 600;

    CShader TotalShader("./shaders/MultiDrawIndirect.vert", "shaders/MultiDrawIndirect.frag");

    // 创建indirect draw缓冲区
    GLuint IndirectBuffer;
    /*glGenBuffers(1, &IndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(SDrawArraysIndirectCommand) * DrawNum, DrawCommand, GL_STATIC_DRAW);*/
    glCreateBuffers(1, &IndirectBuffer);
    glNamedBufferStorage(IndirectBuffer, sizeof(SDrawArraysIndirectCommand) * DrawNum, DrawCommand, GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectBuffer);

    while (!glfwWindowShouldClose(pWindow))
    {
        float CurrentFrame = static_cast<float>(glfwGetTime());
        DeltaTime = CurrentFrame - LastFrame;
        LastFrame = CurrentFrame;
        processInput(pWindow);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.2f, 0.1f, 1.0f);
        TotalShader.use();
        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 View = Camera.GetViewMatrix();
        glm::mat4 Projection = glm::perspective(glm::radians(Camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        TotalShader.setMat4("model", Model);
        TotalShader.setMat4("view", View);
        TotalShader.setMat4("projection", Projection);
        glBindVertexArray(TotalVAO);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, DrawNum, 0);
        //glMultiDrawArraysIndirect(GL_TRIANGLES, DrawCommand, DrawNum, 0);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &TotalVAO);
    glDeleteBuffers(1, &TotalVBO);
    glDeleteBuffers(1, &TotalEBO);
    glDeleteBuffers(1, &IndirectBuffer);

    glfwTerminate();
    return 0;
}