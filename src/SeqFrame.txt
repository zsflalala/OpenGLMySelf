#include <iostream>
#include <utility>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>

#define M_PI 3.14159265358979323846

const char* pSnowVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    uniform vec2  screenScale;
    uniform vec2  screenOffset;
    uniform float rotationAngle;

    out vec2 TexCoord;

    void main() 
    {
        vec2 scalePos = vec2(
            aPos.x * screenScale.x,
            aPos.y * screenScale.y
        );
        
        float cosTheta = cos(rotationAngle);
        float sinTheta = sin(rotationAngle);

        mat2 rotationMatrix = mat2(
            cosTheta, -sinTheta,
            sinTheta,  cosTheta
        );

        vec2 rotatedPos = rotationMatrix * scalePos;

        vec2 transformedPos = vec2(
            rotatedPos.x + screenOffset.x,
            rotatedPos.y + screenOffset.y
        );

        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* pSnowFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;
    uniform vec2 uvOffset;
    uniform vec2 uvScale; 
    uniform sampler2D snowTexture;

    void main()
    {
        vec2 TexCoords = (TexCoord * uvScale) + uvOffset;
        vec4 SnowColor = texture(snowTexture, TexCoords);
        FragColor = SnowColor;
    }
)";

const char* pQuadVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main() 
    {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* pQuadFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;
    uniform sampler2D quadTexture;

    void main()
    {
        vec4 CartoonColor = texture(quadTexture, TexCoord);
        FragColor = CartoonColor;
    }
)";

GLuint compileShader(GLenum vType, const char* vSource)
{
    GLuint Shader = glCreateShader(vType);
    glShaderSource(Shader, 1, &vSource, NULL);
    glCompileShader(Shader);

    int Success;
    char pInfoLog[512];
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(Shader, 512, NULL, pInfoLog);
        std::cerr << "Error: Shader compilation failed\n" << pInfoLog << std::endl;
    }
    return Shader;
}

GLuint loadTexture(const char* vPath)
{
    GLuint TextureID = 0;
    int Width, Height, Channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* pData = stbi_load(vPath, &Width, &Height, &Channels, 0);
    std::cout << vPath << " width: " << Width << " height: " << Height << " Channels: " << Channels << std::endl;
    if (pData)
    {
        GLenum Format = GL_RGB;
        if (Channels == 1)
            Format = GL_RED;
        else if (Channels == 3)
            Format = GL_RGB;
        else if (Channels == 4)
            Format = GL_RGBA;

        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, pData);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << vPath << std::endl;
    }
    stbi_image_free(pData);
    return TextureID;
}

GLuint linkProgram(GLuint vVertShaderHandle, GLuint vFragShaderHandle)
{
    GLuint ProgramHandle = glCreateProgram();
    if (ProgramHandle != 0)
    {
        glAttachShader(ProgramHandle, vVertShaderHandle);
        glAttachShader(ProgramHandle, vFragShaderHandle);
        glLinkProgram(ProgramHandle);
        GLint Result = GL_FALSE;
        glGetProgramiv(ProgramHandle, GL_LINK_STATUS, &Result);
        if (Result == GL_FALSE)
        {
            glDeleteProgram(ProgramHandle);
            return 0;
        }
        return ProgramHandle;
    }
    glDeleteShader(vVertShaderHandle);
    glDeleteShader(vFragShaderHandle);
    return 0;
}

bool  g_EnableFarSnow = true;
bool  g_EnableNearSnow = true;
bool  g_EnableCartoon = true;
bool  g_EnableBackground = true;
float g_FarSnowSpeed = 24.0f;
float g_NearSnowSpeed = 24.0f;
float g_SnowSceneSpeed = 8.0f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    constexpr float DeltaSpeed = 5.0f;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        g_EnableFarSnow = !g_EnableFarSnow;
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        g_EnableNearSnow = !g_EnableNearSnow;
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
        g_EnableCartoon = !g_EnableCartoon;
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
        g_EnableBackground = !g_EnableBackground;
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        g_FarSnowSpeed = std::max(1.0f, g_FarSnowSpeed - DeltaSpeed);
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
        g_FarSnowSpeed = std::min(100.0f, g_FarSnowSpeed + DeltaSpeed);
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        g_NearSnowSpeed = std::max(1.0f, g_NearSnowSpeed - DeltaSpeed);
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
        g_NearSnowSpeed = std::min(100.0f, g_NearSnowSpeed + DeltaSpeed);
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
        g_SnowSceneSpeed = std::max(1.0f, g_SnowSceneSpeed - DeltaSpeed);
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
        g_SnowSceneSpeed = std::min(100.0f, g_SnowSceneSpeed + DeltaSpeed);
}

void windowSizeCallback(GLFWwindow* vWindow, int vWidth, int vHeight)
{
    glViewport(0, 0, vWidth, vHeight);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int WindowWidth = 1400, WindowHeight = 630;
    GLFWwindow* pWindow = glfwCreateWindow(WindowWidth, WindowHeight, "Sequence Frames", NULL, NULL);
    if (!pWindow)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(pWindow);
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetFramebufferSizeCallback(pWindow, windowSizeCallback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    double CurrentTime = glfwGetTime();
    GLuint NearSnowTextureHandle = loadTexture("./Textures/nearSnow.png");
    double LoadNearTextureTime = glfwGetTime();
    std::cout << LoadNearTextureTime - CurrentTime << std::endl;
    GLuint FarSnowTextureHandle = loadTexture("./Textures/farSnow.png");
    double LoadFarTextureTime = glfwGetTime();
    std::cout << LoadFarTextureTime - LoadNearTextureTime << std::endl;
    //GLuint CartoonTextureHandle = loadTexture("./Textures/Deliverman.png");
    GLuint SceneTextureHandle   = loadTexture("./Textures/snowScene.png");
    GLuint BackgroundTextureHandle = loadTexture("./Textures/background3.jpg");

    GLuint NearSnowVertexShader = compileShader(GL_VERTEX_SHADER, pSnowVertexShaderSource);
    GLuint NearSnowFragmentShader = compileShader(GL_FRAGMENT_SHADER, pSnowFragmentShaderSource);
    GLuint NearSnowShaderProgram = linkProgram(NearSnowVertexShader, NearSnowFragmentShader);

    GLuint SnowSceneVertexShader = compileShader(GL_VERTEX_SHADER, pSnowVertexShaderSource);
    GLuint SnowSceneFragmentShader = compileShader(GL_FRAGMENT_SHADER, pSnowFragmentShaderSource);
    GLuint SnowSceneShaderProgram = linkProgram(SnowSceneVertexShader, SnowSceneFragmentShader);

    GLuint CartoonVertexShader = compileShader(GL_VERTEX_SHADER, pQuadVertexShaderSource);
    GLuint CartoonFragmentShader = compileShader(GL_FRAGMENT_SHADER, pQuadFragmentShaderSource);
    GLuint CartoonShaderProgram = linkProgram(CartoonVertexShader, CartoonFragmentShader);

    GLuint BackgroundVertexShader = compileShader(GL_VERTEX_SHADER, pQuadVertexShaderSource);
    GLuint BackgroundFragmentShader = compileShader(GL_FRAGMENT_SHADER, pQuadFragmentShaderSource);
    GLuint BackgroundShaderProgram = linkProgram(BackgroundVertexShader, BackgroundFragmentShader);

    GLuint FarSnowVertexShader = compileShader(GL_VERTEX_SHADER, pSnowVertexShaderSource);
    GLuint FarSnowFragmentShader = compileShader(GL_FRAGMENT_SHADER, pSnowFragmentShaderSource);
    GLuint FarSnowShaderProgram = linkProgram(FarSnowVertexShader, FarSnowFragmentShader);

    const float pVertices[] = {
        // positions   // texCoords
        -1.0f, -1.0f,  0.0f, 0.0f, // bottom left
         1.0f, -1.0f,  1.0f, 0.0f, // bottom right
         1.0f,  1.0f,  1.0f, 1.0f, // top right
        -1.0f,  1.0f,  0.0f, 1.0f  // top left
    };

    const unsigned int pIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLuint QuadVAO, QuadVBO, QuadEBO;
    glGenVertexArrays(1, &QuadVAO);
    glBindVertexArray(QuadVAO);
    glGenBuffers(1, &QuadVBO);
    glGenBuffers(1, &QuadEBO);
    glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pVertices), pVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndices), pIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    double NearLastTime = glfwGetTime();
    double FarLastTime  = glfwGetTime();
    int NearCurrentFrame = 0;
    int FarCurrentFrame  = 0;
    const int NearRows   = 8, NearCols = 16;
    const int FarRows    = 8, FarCols  = 16;
    const float Angle = 0.0f * M_PI / 180.0f; // Clockwise
    const glm::vec2 ScreenScale = glm::vec2(1.0f, 1.0f);
    const glm::vec2 ScreenOffset = glm::vec2(0.0f, 0.0f);
    int NearSnowValidFrames = NearRows * NearCols;
    int FarSnowValidFrames  = FarRows  * FarCols;

    double SnowSceneLastTime = glfwGetTime();
    const int SnowSceneRows = 8, SnowSceneCols = 16;
    int SnowSceneCurrentFrame = 0;
    int SnowSceneValidFrames = SnowSceneRows * SnowSceneCols;
    bool IsFinished = false;

    while (!glfwWindowShouldClose(pWindow))
    {
        double CurrentTime = glfwGetTime();

        double NearFrameTime = 1.0 / g_NearSnowSpeed;
        if (CurrentTime - NearLastTime >= NearFrameTime)
        {
            NearLastTime = CurrentTime;
            NearCurrentFrame = (NearCurrentFrame + 1) % NearSnowValidFrames;
        }

        double FarFrameTime = 1.0 / g_FarSnowSpeed;
        if (CurrentTime - FarLastTime >= FarFrameTime)
        {
            FarLastTime = CurrentTime;
            FarCurrentFrame = (FarCurrentFrame + 1) % FarSnowValidFrames;
        }

        double SnowSceneFrameTime = 1.0 / g_SnowSceneSpeed;
        if (CurrentTime - SnowSceneLastTime >= SnowSceneFrameTime)
        {
            SnowSceneLastTime = CurrentTime;
            SnowSceneCurrentFrame = (SnowSceneCurrentFrame + 1) % SnowSceneValidFrames;
            if (SnowSceneCurrentFrame == 0) IsFinished = true;
            else IsFinished = false;
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);

        //if (g_EnableBackground)
        //{
        //    glUseProgram(BackgroundShaderProgram);
        //    glUniform1i(glGetUniformLocation(BackgroundShaderProgram, "quadTexture"), 0);
        //    glBindVertexArray(QuadVAO);
        //    glBindTexture(GL_TEXTURE_2D, BackgroundTextureHandle);
        //    glActiveTexture(GL_TEXTURE0);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //}

        /*if (g_EnableFarSnow)
        {
            int  FarRow = FarCurrentFrame / FarCols;
            int  FarCol = FarCurrentFrame % FarCols;
            float FarU0 = FarCol / (float)FarCols;
            float FarV0 = FarRow / (float)FarRows;
            float FarU1 = (FarCol + 1) / (float)FarCols;
            float FarV1 = (FarRow + 1) / (float)FarRows;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUseProgram(FarSnowShaderProgram);
            glUniform1f(glGetUniformLocation(FarSnowShaderProgram, "rotationAngle"), Angle);
            glUniform2fv(glGetUniformLocation(FarSnowShaderProgram, "screenScale"), 1, &ScreenScale[0]);
            glUniform2fv(glGetUniformLocation(FarSnowShaderProgram, "screenOffset"), 1, &ScreenOffset[0]);
            glUniform2f(glGetUniformLocation(FarSnowShaderProgram, "uvOffset"), FarU0, FarV0);
            glUniform2f(glGetUniformLocation(FarSnowShaderProgram, "uvScale"), FarU1 - FarU0, FarV1 - FarV0);
            glUniform1i(glGetUniformLocation(FarSnowShaderProgram, "snowTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, FarSnowTextureHandle);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(QuadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }*/
        
        if (g_EnableCartoon)
        {
            if (IsFinished) SnowSceneCurrentFrame = SnowSceneValidFrames - 1;
            int SnowSceneRow = SnowSceneCurrentFrame / SnowSceneCols;
            int SnowSceneCol = SnowSceneCurrentFrame % SnowSceneCols;
            float SnowSceneU0 = SnowSceneCol / (float)SnowSceneCols;
            float SnowSceneV0 = 1.0 - (SnowSceneRow + 1) / (float)SnowSceneRows;
            float SnowSceneU1 = (SnowSceneCol + 1) / (float)SnowSceneCols;
            float SnowSceneV1 = 1.0 - SnowSceneRow / (float)SnowSceneRows;

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUseProgram(SnowSceneShaderProgram);
            glUniform1f(glGetUniformLocation(SnowSceneShaderProgram, "rotationAngle"), Angle);
            glUniform2fv(glGetUniformLocation(SnowSceneShaderProgram, "screenScale"), 1, &ScreenScale[0]);
            glUniform2fv(glGetUniformLocation(SnowSceneShaderProgram, "screenOffset"), 1, &ScreenOffset[0]);
            glUniform2f(glGetUniformLocation(SnowSceneShaderProgram, "uvOffset"), SnowSceneU0, SnowSceneV0);
            glUniform2f(glGetUniformLocation(SnowSceneShaderProgram, "uvScale"), SnowSceneU1 - SnowSceneU0, SnowSceneV1 - SnowSceneV0);
            glUniform1i(glGetUniformLocation(SnowSceneShaderProgram, "snowTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, SceneTextureHandle);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(QuadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        if (g_EnableNearSnow)
        {
            int NearRow = NearCurrentFrame / NearCols;
            int NearCol = NearCurrentFrame % NearCols;
            float NearU0 = NearCol / (float)NearCols;
            float NearV0 = NearRow / (float)NearRows;
            float NearU1 = (NearCol + 1) / (float)NearCols;
            float NearV1 = (NearRow + 1) / (float)NearRows;

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUseProgram(NearSnowShaderProgram);
            glUniform1f(glGetUniformLocation(NearSnowShaderProgram, "rotationAngle"), Angle);
            glUniform2fv(glGetUniformLocation(NearSnowShaderProgram, "screenScale"), 1, &ScreenScale[0]);
            glUniform2fv(glGetUniformLocation(NearSnowShaderProgram, "screenOffset"), 1, &ScreenOffset[0]);
            glUniform2f(glGetUniformLocation(NearSnowShaderProgram, "uvOffset"), NearU0, NearV0);
            glUniform2f(glGetUniformLocation(NearSnowShaderProgram, "uvScale"), NearU1 - NearU0, NearV1 - NearV0);
            glUniform1i(glGetUniformLocation(NearSnowShaderProgram, "snowTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, NearSnowTextureHandle);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(QuadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &QuadVBO);
    glDeleteBuffers(1, &QuadEBO);
    glDeleteProgram(NearSnowShaderProgram);
    glDeleteProgram(FarSnowShaderProgram);
    glDeleteProgram(CartoonShaderProgram);
    glDeleteProgram(BackgroundShaderProgram);
    glfwTerminate();
    return 0;
}