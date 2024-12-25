#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <stb_image.h>

#define FRAME_COUNT 128

GLuint compileShader(GLenum vShaderType, const char* vShaderSource)
{
    GLuint Shader = glCreateShader(vShaderType);
    glShaderSource(Shader, 1, &vShaderSource, nullptr);
    glCompileShader(Shader);

    GLint Success;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        GLint LogLength;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);
        std::string Log(LogLength, '\0');
        glGetShaderInfoLog(Shader, LogLength, nullptr, Log.data());
        std::cerr << "Shader compile error: " << Log << std::endl;
    }
    return Shader;
}

GLuint createShaderProgram()
{
    const char* pVertexShaderSource = R"(
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

    const char* pFragmentShaderSource = R"(
	    #version 330 core
	    out vec4 FragColor;

	    in vec2 TexCoord;
	    uniform sampler2D snowTexture;

	    void main()
	    {
	        vec4 SnowColor = texture(snowTexture, TexCoord);
	        FragColor = SnowColor;
	    }
	)";

    GLuint VertexShader = compileShader(GL_VERTEX_SHADER, pVertexShaderSource);
    GLuint FragmentShader = compileShader(GL_FRAGMENT_SHADER, pFragmentShaderSource);

    GLuint Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glLinkProgram(Program);

    GLint Success;
    glGetProgramiv(Program, GL_LINK_STATUS, &Success);
    if (!Success)
    {
        GLint LogLength;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);
        std::string Log(LogLength, '\0');
        glGetProgramInfoLog(Program, LogLength, nullptr, Log.data());
        std::cerr << "Program linking error: " << Log << std::endl;
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    return Program;
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        exit(-1);
    }
	GLFWwindow* pWindow = glfwCreateWindow(800, 600, "OpenGL Texture Playback", nullptr, nullptr);
    if (!pWindow)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(pWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        exit(-1);
    }
    GLFWwindow* pSharedWindow = glfwCreateWindow(1, 1, "Shared Context", nullptr, pWindow);
    if (!pSharedWindow) 
    {
        std::cerr << "Failed to create shared context window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLuint ShaderProgram = createShaderProgram();

    float pQuadVertices[] = {
        // positions              // texCoords
        -1.0f,  1.0f, 0.0f, 0.0f, // bottom left
         1.0f,  1.0f, 1.0f, 0.0f, // bottom right
         1.0f, -1.0f, 1.0f, 1.0f, // top right
        -1.0f, -1.0f, 0.0f, 1.0f  // top left
    };

    unsigned int pIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pQuadVertices), pQuadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndices), pIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    std::mutex TextureMutex;
    std::atomic<bool> pFrameLoaded[FRAME_COUNT] = {false};
    GLuint pTextureIDs[FRAME_COUNT];
    glGenTextures(FRAME_COUNT, pTextureIDs);
    for (int i = 0; i < FRAME_COUNT; ++i)
    {
        std::string TexturePath = "Textures/SnowScene/frame_" + std::string(3 - std::to_string(i + 1).length(), '0') + std::to_string(i + 1) + ".png";
        std::thread([pSharedWindow, &pTextureIDs, &TextureMutex, &pFrameLoaded, i, TexturePath]()
            {
                std::lock_guard<std::mutex> Lock(TextureMutex); // If multiple threads call OpenGL functions simultaneously and the OpenGL context is not locked properly, inconsistencies or errors may result.
                glfwMakeContextCurrent(pSharedWindow);
                int Width, Height, Channels;
                unsigned char* pImageData = stbi_load(TexturePath.c_str(), &Width, &Height, &Channels, 0);
                if (pImageData)
                {
                    glBindTexture(GL_TEXTURE_2D, pTextureIDs[i]);
                    GLenum format = (Channels == 4) ? GL_RGBA : GL_RGB;
                    glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, format, GL_UNSIGNED_BYTE, pImageData);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    pFrameLoaded[i].store(true);
                    stbi_image_free(pImageData);
                }
                else
                {
                    pFrameLoaded[i].store(false);
                    std::cerr << "Failed to load texture: " << TexturePath << std::endl;
                }
                glfwMakeContextCurrent(nullptr);
            }).detach(); // The join method forces the main thread to wait, while the detach method does not.
    }

    int  Frame = 0;
    int  LastLoadedFrame = -1;                                   // Save the last loaded frame
    auto LastFrameTime   = std::chrono::steady_clock::now();     // Record the time of the last frame rendering
    auto Threshold       = std::chrono::milliseconds(100);   // Set the maximum wait time to 100 milliseconds (0.1 seconds)

    while (!glfwWindowShouldClose(pWindow)) 
    {
        glClearColor(0.1f, 0.2f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);

        auto Now = std::chrono::steady_clock::now();

        // If the current frame is not loaded, determine whether the time exceeds the threshold
        if (pFrameLoaded[Frame].load()) 
        {
            LastLoadedFrame = Frame; // Update the last loaded frame
            Frame = (Frame + 1) % FRAME_COUNT;
            LastFrameTime = Now;     // Update last render time
        }
        else
        {
            auto TimeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastFrameTime);

            // If the threshold is exceeded, skip the current frame
            if (TimeElapsed > Threshold) 
            {
                std::cout << "Frame " << Frame << " is not loaded for too long, skipping to next frame." << std::endl;
                Frame = (Frame + 1) % FRAME_COUNT;
                LastFrameTime = Now;
            }
            else if (LastLoadedFrame != -1)
            {
                std::cout << "Frame " << Frame << " is not loaded, showing last loaded frame: " << LastLoadedFrame << std::endl;
            }
            else 
            {
                std::cerr << "No frame is loaded yet." << std::endl;
                glfwSwapBuffers(pWindow);
                glfwPollEvents();
                continue;
            }
        }

        // Render the last valid frame (or the current frame)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(ShaderProgram);
        glUniform1i(glGetUniformLocation(ShaderProgram, "snowTexture"), 0);
        glBindTexture(GL_TEXTURE_2D, pTextureIDs[LastLoadedFrame]);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glfwDestroyWindow(pWindow);
    glfwTerminate();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    return 0;
}