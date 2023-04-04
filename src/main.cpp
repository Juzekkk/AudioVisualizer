#include <iostream>
#include <vector>
#include <windows.h>
#include "AudioCapture.h"
#include "AudioProcessor.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void clearLine()
{
    std::cout << "\033[2K";
}

void moveCursorUp(int lines)
{
    std::cout << "\033[" << lines << "A";
}

void processAudioData(BYTE *pData, LONG dataSize, const WAVEFORMATEX &format, AudioProcessor &audioProcessor)
{
    std::vector<double> magnitudes = audioProcessor.process(pData, dataSize, format);
    int scale = 5; // Adjust this value to change the number of '#' signs used for the maximum magnitude

    for (size_t i = 0; i < magnitudes.size(); ++i)
    {
        clearLine();
        std::cout << "Frequency #" << i << ": ";

        int barLength = static_cast<int>(magnitudes[i] * scale);
        for (int j = 0; j < barLength; ++j)
        {
            std::cout << "#";
        }

        std::cout << std::endl;
    }

    moveCursorUp(magnitudes.size());
}

void enableVirtualTerminalProcessing()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

int main()
{
    AudioCapture audioCapture;
    audioCapture.initialize();

    AudioProcessor audioProcessor(16);

    audioCapture.setCallback([&](BYTE *pData, LONG dataSize)
                             { processAudioData(pData, dataSize, *audioCapture.getFormat(), audioProcessor); });

    audioCapture.startCapture();

    return 0;
}

// int main()
// {
//     if (!glfwInit())
//     {
//         std::cerr << "Failed to initialize GLFW" << std::endl;
//         return -1;
//     }

//     GLFWwindow *window = glfwCreateWindow(1280, 720, "ImGui Test1", nullptr, nullptr);
//     if (!window)
//     {
//         std::cerr << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }

//     glfwMakeContextCurrent(window);
//     glfwSwapInterval(1);

//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cerr << "Failed to initialize OpenGL loader" << std::endl;
//         return -1;
//     }

//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO &io = ImGui::GetIO();
//     (void)io;
//     ImGui::StyleColorsDark();
//     ImGui_ImplGlfw_InitForOpenGL(window, true);
//     ImGui_ImplOpenGL3_Init("#version 130");

//     while (!glfwWindowShouldClose(window))
//     {
//         glfwPollEvents();

//         ImGui_ImplOpenGL3_NewFrame();
//         ImGui_ImplGlfw_NewFrame();
//         ImGui::NewFrame();

//         ImGui::Begin("Hello, Smyks here!");
//         ImGui::Text("This is a simple example using Dear ImGui.");
//         ImGui::End();

//         ImGui::Render();
//         int display_w, display_h;
//         glfwGetFramebufferSize(window, &display_w, &display_h);
//         glViewport(0, 0, display_w, display_h);
//         glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
//         glClear(GL_COLOR_BUFFER_BIT);
//         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//         glfwSwapBuffers(window);
//     }

//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplGlfw_Shutdown();
//     ImGui::DestroyContext();

//     glfwDestroyWindow(window);
//     glfwTerminate();

//     return 0;
// }