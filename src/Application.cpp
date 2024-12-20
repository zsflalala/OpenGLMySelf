#include <iostream>
#include <utility>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
	#version 430 core
	out vec4 FragColor;

	in vec2 TexCoord;
	uniform vec2 texUVOffset;
	uniform vec2 texUVScale; 
	uniform sampler2D sequenceTexture;
	uniform sampler2D sequenceTextureA;
	uniform sampler2D sequenceTextureB;

	uniform vec2 preTexUVOffset;
	uniform vec2 preTexUVScale;
	uniform float interpolationParam;

	void main()
	{
	    vec3 LightDir = normalize(vec3(-2.0f, -3.0f, -5.0f));
	    vec3 LightColor = vec3(1.0f, 1.0f, 1.0f);

	    float Intensity = 1.0f;

	    vec2 TexCoords = (TexCoord * texUVScale) + texUVOffset;
	    vec4 TexColor = texture(sequenceTexture, TexCoords);
	    vec4 TexColorA = texture(sequenceTextureA, TexCoords);
	    vec4 TexColorB = texture(sequenceTextureB, TexCoords);

	    vec2 PreTexCoords = (TexCoord * preTexUVScale) + preTexUVOffset;
	    vec4 PreTexColor = texture(sequenceTexture, PreTexCoords);
	    vec4 PreTexColorA = texture(sequenceTextureA, PreTexCoords);
	    vec4 PreTexColorB = texture(sequenceTextureB, PreTexCoords);

	    LightDir = abs(LightDir);

	    float Gray = TexColorA.r * LightDir.x + TexColorA.g * LightDir.y + TexColorB.b * LightDir.z;
	    float PreGray = PreTexColorA.r * LightDir.x + PreTexColorA.g * LightDir.y + PreTexColorB.b * LightDir.z;
	    vec4 CurrentColor = vec4(Gray, Gray, Gray, 1.0f);
	    vec4 PreColor = vec4(PreGray, PreGray, PreGray, 1.0f);

	    TexColor = TexColor * CurrentColor * Intensity;
	    PreTexColor = PreTexColor * PreColor * Intensity;

	    FragColor = mix(PreTexColor, TexColor, interpolationParam);
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

    const int WindowWidth = 1920, WindowHeight = 1080;
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

    GLuint SceneTextureHandle = loadTexture("./Textures/cloud2_12x15.png");
    GLuint SceneTextureHandleA = loadTexture("./Textures/cloud2_12x15_A.png");
    GLuint SceneTextureHandleB = loadTexture("./Textures/cloud2_12x15_B.png");

    GLuint SceneVertexShader = compileShader(GL_VERTEX_SHADER, pSnowVertexShaderSource);
    GLuint SceneFragmentShader = compileShader(GL_FRAGMENT_SHADER, pSnowFragmentShaderSource);
    GLuint SceneShaderProgram = linkProgram(SceneVertexShader, SceneFragmentShader);

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

    const float Angle = 0.0f * M_PI / 180.0f; // Clockwise
    const glm::vec2 ScreenScale = glm::vec2(1.0f, 1.0f);
    const glm::vec2 ScreenOffset = glm::vec2(0.0f, 0.0f);

    double SceneLastTime = glfwGetTime();
    const int SnowSceneRows = 12, SnowSceneCols = 15;
    int SceneCurrentFrame = 0;
    int SceneValidFrames = SnowSceneRows * SnowSceneCols;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& Io = ImGui::GetIO();
    Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    Io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(pWindow, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 430");

    bool ShowDemoWindow = false;
    bool ShowAnotherWindow = false;
    ImVec4 ClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(pWindow))
    {
        double CurrentTime = glfwGetTime();

        double SceneFrameTime = 1.0 / g_SnowSceneSpeed;
        if (CurrentTime - SceneLastTime >= SceneFrameTime)
        {
            SceneLastTime = CurrentTime;
            SceneCurrentFrame = (SceneCurrentFrame + 1) % SceneValidFrames;
        }

        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static float InterpolationParam = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &ShowDemoWindow);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &ShowAnotherWindow);
        if (ShowDemoWindow) ImGui::ShowDemoWindow(&ShowDemoWindow);
        ImGui::SliderFloat("float", &InterpolationParam, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&ClearColor); // Edit 3 floats representing a color
        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / Io.Framerate, Io.Framerate);
        ImGui::End();
        if (ShowAnotherWindow)
        {
            ImGui::Begin("Another Window");
            ImGui::Text("This is another text.");
            if (ImGui::Button("Close Me")) ShowAnotherWindow = false;
            ImGui::End();
        }

        int SceneRow = SceneCurrentFrame / SnowSceneCols;
        int SceneCol = SceneCurrentFrame % SnowSceneCols;
        float SceneU0 = SceneCol / (float)SnowSceneCols;
        float SceneV0 = 1.0 - (SceneRow + 1) / (float)SnowSceneRows;
        float SceneU1 = (SceneCol + 1) / (float)SnowSceneCols;
        float SceneV1 = 1.0 - SceneRow / (float)SnowSceneRows;

        int   PreFrame = std::max(SceneCurrentFrame - 1, 0);
        int   PreFrameRow = PreFrame / SnowSceneCols;
        int   PreFrameCol = PreFrame % SnowSceneCols;
        float PreFrameU0 = PreFrameCol / static_cast<float>(SnowSceneCols);
        float PreFrameV0 = 1.0 - (PreFrameRow + 1) / static_cast<float>(SnowSceneRows);
        float PreFrameU1 = (PreFrameCol + 1) / static_cast<float>(SnowSceneCols);
        float PreFrameV1 = 1.0 - PreFrameRow / static_cast<float>(SnowSceneRows);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(SceneShaderProgram);
        glUniform1f(glGetUniformLocation(SceneShaderProgram, "rotationAngle"), Angle);
        glUniform2fv(glGetUniformLocation(SceneShaderProgram, "screenScale"), 1, &ScreenScale[0]);
        glUniform2fv(glGetUniformLocation(SceneShaderProgram, "screenOffset"), 1, &ScreenOffset[0]);
        glUniform2f(glGetUniformLocation(SceneShaderProgram, "texUVOffset"), SceneU0, SceneV0);
        glUniform2f(glGetUniformLocation(SceneShaderProgram, "texUVScale"), SceneU1 - SceneU0, SceneV1 - SceneV0);
        glUniform2f(glGetUniformLocation(SceneShaderProgram, "preTexUVOffset"), PreFrameU0, PreFrameV0);
        glUniform2f(glGetUniformLocation(SceneShaderProgram, "preTexUVScale"), PreFrameU1 - PreFrameU0, PreFrameV1 - PreFrameV0);
        glUniform1f(glGetUniformLocation(SceneShaderProgram, "interpolationParam"), InterpolationParam);

        glUniform1i(glGetUniformLocation(SceneShaderProgram, "sequenceTexture"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SceneTextureHandle);
        glUniform1i(glGetUniformLocation(SceneShaderProgram, "sequenceTextureA"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, SceneTextureHandleA);
        glUniform1i(glGetUniformLocation(SceneShaderProgram, "sequenceTextureB"), 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, SceneTextureHandleB);
        glBindVertexArray(QuadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &QuadVBO);
    glDeleteBuffers(1, &QuadEBO);
    glDeleteProgram(SceneShaderProgram);
    glfwTerminate();
    return 0;
}