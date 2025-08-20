#include <cstdlib>
#include <ctime>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// ImGui includes
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Game Includes
#include <shader_playground/graphics/shader.hpp>
#include <shader_playground/gui/core.hpp>

using namespace std;

static int WINDOW_WIDTH = 1920;
static int WINDOW_HEIGHT = 1080;

static double MOUSE_X, MOUSE_Y = 0;

static bool GUI_ENABLED = true;
static bool G_KEYSTATES[GLFW_KEY_LAST + 1] = {false};

static char DEFAULT_FRAGMENT_DATA[65536] = R"(#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

void main()
{
    FragColor = vec4(TexCoord, 0.0, 1.0);
})";
static char SAVEFILE_LOCATION[256] = R"(shaderSaves.ss1)";

static float deltaTime = 0.;
static float totalTime = 0;

static float clearColor[4] = {.0f};

void processInput(GLFWwindow *window);
int saveShaderTextToFile(char data[65536], unsigned char savePosition);
int loadShaderFromFile(unsigned char savePosition, char* writeTo);

int main() {
    cout << "Running..." << std::endl;
    srand(time(0));

    // --------------------
    // Init GLFW
    // --------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Shader Playground", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        WINDOW_WIDTH=width;
        WINDOW_HEIGHT=height;
        glViewport(0,0, width, height);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y){
        MOUSE_X = x;
        MOUSE_Y = y;
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        if (key == GLFW_KEY_F1 && action == GLFW_PRESS && !G_KEYSTATES[key])
        {
            GUI_ENABLED = !GUI_ENABLED;
        }

        // manage key states
        if (action == GLFW_PRESS)
        {
            G_KEYSTATES[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            G_KEYSTATES[key] = false;
        }
    });

    // --------------------
    // Init ImGui
    // --------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // --------------------
    // Init Variables
    // --------------------

    unsigned char currentShaderSaveIndex = 0;

    // Core

    float previousTime = glfwGetTime();
    static char shaderCodeBuffer[65536];
    strncpy(shaderCodeBuffer, DEFAULT_FRAGMENT_DATA, sizeof(shaderCodeBuffer));

    Shader shaderPreview = Shader(shaderCodeBuffer);

    // Init shader

    float vertices[] = {
        // positions         // texCoords
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,  // bottom left
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // bottom right
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,  // top right

        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,  // bottom left
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,  // top right
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f   // top left
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --------------------
    // Functions
    // --------------------

    static std::function<void(char*)> compileShader = [&shaderPreview](char* data) {
        shaderPreview = Shader(data);
    };

    // --------------------
    // Declare Frames
    // --------------------

    UIFrame infoFrame("INFO", [&previousTime, &window]() {
        ImGui::Text("u_delta: %.8f", deltaTime);
        ImGui::Text("u_time: %.8f", totalTime);
        ImGui::Text("u_mouse: (%.8f, %.8f)", MOUSE_X / WINDOW_WIDTH, MOUSE_Y / WINDOW_HEIGHT);
        ImGui::Text("u_resolution: (%i, %i)", WINDOW_WIDTH, WINDOW_HEIGHT);
        if (ImGui::Button("Exit"))
        {
            glfwSetWindowShouldClose(window, true);
        }
    }, false);

    UIFrame shaderMakerFrame("SHADER MAKER", [&currentShaderSaveIndex](){
        ImGui::InputTextMultiline("Fragment Shader Code", shaderCodeBuffer, sizeof(shaderCodeBuffer), ImVec2(0,160));
        if (ImGui::Button("Compile"))
        {
            compileShader(shaderCodeBuffer);
        }
        ImGui::InputScalar("Save Position", ImGuiDataType_U8, &currentShaderSaveIndex);
        if (ImGui::Button("Save Shader"))
        {
            saveShaderTextToFile(shaderCodeBuffer, currentShaderSaveIndex);
        }
        if (ImGui::Button("Load Shader"))
        {
            loadShaderFromFile(currentShaderSaveIndex, shaderCodeBuffer);
        }
    }, false);

    // UIFrame shaderInfoFrame("SHADER INFO", [](){
    //     ImGui::TextWrapped("");
    // }, false);

    static vector<UIFrame*> frames = {&infoFrame, &shaderMakerFrame};

    // --------------------
    // Main Loop
    // --------------------
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        float currentTime = glfwGetTime();
        deltaTime = (currentTime - previousTime);
        previousTime = currentTime;
        totalTime += deltaTime;

        if (GUI_ENABLED)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            for (UIFrame *f : frames)
            {
                f->draw();
            }

            // Rendering
            ImGui::Render();
        }
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderPreview.use();
        shaderPreview.setFloat("u_time", totalTime);
        shaderPreview.setFloat("u_delta", deltaTime);
        shaderPreview.setVec2("u_mouse", MOUSE_X / WINDOW_WIDTH, MOUSE_Y / WINDOW_HEIGHT);
        shaderPreview.setVec2("u_mousep", MOUSE_X, MOUSE_Y);

        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            shaderPreview.setVec2("u_resolution", width, height);
        }
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        if (GUI_ENABLED) ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --------------------
    // Cleanup
    // --------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
int saveShaderTextToFile(char data[65536], unsigned char savePosition)
{
    std::fstream saveFile;
    long long position = 65536 * savePosition;
    std::fstream file("shaderSaves.ss1", std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open shader savefile!" << std::endl;
        return 1;
    }

    file.seekg(position);

    file.write(data, 65536);

    file.close();

    std::cout << "Shader saved at ID <" << static_cast<unsigned int>(savePosition) << "> (" << position << ")!" << std::endl;

    return 0;
}
int loadShaderFromFile(unsigned char savePosition, char* writeTo)
{
    std::fstream saveFile;
    long long position = 65536 * savePosition;
    std::fstream file("shaderSaves.ss1", std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open shader savefile!" << std::endl;
        return 1;
    }

    file.seekg(position);

    file.read(writeTo, 65536);

    file.close();
    std::cout << "Shader loaded at ID <" << static_cast<unsigned int>(savePosition) << "> (" << position << ")!" << std::endl;
    return 0;
}
