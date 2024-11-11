#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "shader.h"

// ���ڴ�С
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void RenderQuad(CShader& vShader);
void RenderScene(CShader& vShader);

int main()
{
    // ��ʼ��GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ��������
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Deferred Rendering", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ��ʼ��GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ����ͼ�����ɫ��
    CShader geometryShader("./shaders/DefferredShading/gbuffer.vs", "./shaders/DefferredShading/gbuffer.fs");
    CShader lightingShader("./shaders/DefferredShading/lighting.vs", "./shaders/DefferredShading/lighting.fs");

    glUniform1i(glGetUniformLocation(lightingShader.ID, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.ID, "gNormal"), 1);

    // ����G-Buffer
    unsigned int gBuffer;
    /*glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);*/
    glCreateFramebuffers(1, &gBuffer);
    // λ����ɫ����

    unsigned int gPosition;
    /*glBindTexture(GL_TEXTURE_2D, gPosition);
    glGenTextures(1, &gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);*/
    glCreateTextures(GL_TEXTURE_2D, 1, &gPosition);
    glTextureStorage2D(gPosition, 1, GL_RGB8, SCR_WIDTH, SCR_HEIGHT);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, gPosition, 0);

    // ������ɫ����
    unsigned int gNormal;
    /*glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);*/
    glCreateTextures(GL_TEXTURE_2D, 1, &gNormal);
    glTextureStorage2D(gNormal, 1, GL_RGB8, SCR_WIDTH, SCR_HEIGHT);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, gNormal, 0);

    // ����OpenGL���ǽ�ʹ��������ɫ����������Ⱦ
    /*unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);*/
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glNamedFramebufferDrawBuffers(gBuffer, 2, attachments);

    // �������
    unsigned int gDepth;
    /*glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);*/
    glCreateTextures(GL_TEXTURE_2D, 1, &gDepth);
    glTextureStorage2D(gDepth, 1, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT);
    glNamedFramebufferTexture(gBuffer, GL_DEPTH_ATTACHMENT, gDepth, 0);

    // ���֡�����Ƿ�����
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightingShader.use();
    lightingShader.setInt("gPosition", 0);
    lightingShader.setInt("gNormal", 1);

    // ��ѭ��
    while (!glfwWindowShouldClose(window))
    {
        // ���봦��
        processInput(window);

        // ���δ���׶�
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        geometryShader.use();
        RenderScene(geometryShader);

        // ���մ���׶�
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /*glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);*/
        glBindTextureUnit(0, gPosition);
        glBindTextureUnit(1, gNormal);
        lightingShader.use();
        RenderQuad(lightingShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ������Դ
    glDeleteFramebuffers(1, &gBuffer);
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gDepth);

    glfwTerminate();
    return 0;
}

// ��������
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// �����ڴ�С�ı�ʱ����
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ��Ⱦ������Ļ���ı���
unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderQuad(CShader& vShader)
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glm::vec3 CameraPos = glm::vec3(0.0f, 0.0f, -3.0f);
    glm::vec3 CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 ViewMat = glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
    vShader.setMat4("viewPos", ViewMat);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int TriangleVAO = 0;
unsigned int TriangleVBO;
void RenderScene(CShader& vShader)
{
    if (TriangleVAO == 0)
    {
        float vertices[] = {
            // λ��           // ����
            0.0f,  0.5f, 0.0f,  0.0f,  0.0f,  1.0f,
           -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,
        };

        glGenVertexArrays(1, &TriangleVAO);
        glBindVertexArray(TriangleVAO);

        glGenBuffers(1, &TriangleVBO);
        glBindBuffer(GL_ARRAY_BUFFER, TriangleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    glm::vec3 CameraPos = glm::vec3(0.0f, 0.0f, -2.0f);
    glm::vec3 CameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 ProjectionMat = glm::perspective(glm::radians(45.0f), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 ViewMat = glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
    vShader.setMat4("projection", ProjectionMat);
    vShader.setMat4("view", ViewMat);
    glm::mat4 Model = glm::mat4(1.0f);
    vShader.setMat4("model", Model);

    glBindVertexArray(TriangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}