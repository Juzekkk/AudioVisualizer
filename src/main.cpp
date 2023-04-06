#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "AudioCapture.h"
#include "AudioProcessor.h"
#include "TransparentWindow.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdlib>

void modifyLogAlternation(std::vector<float> &vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = std::log(vec[i] + 1) / std::log(100);
    }
}

int main()
{
    // Initialize audio capture and processing
    AudioCapture audioCapture;

    if (!audioCapture.initialize())
    {
        std::cerr << "Failed to initialize AudioCapture" << std::endl;
        return 1;
    }

    audioCapture.startCapture();

    unsigned int numberOfWindows = 12; // Change this to the desired number of frequency windows
    AudioProcessor audioProcessor(numberOfWindows, audioCapture);
    audioProcessor.startProcessing();

    const unsigned int displayIntervalMs = 10;

    // Initialize the transparent window
    TransparentWindow transparentWindow;
    transparentWindow.createWindow();
    GLFWwindow *window = transparentWindow.getWindow();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    while (!glfwWindowShouldClose(window))
    {

        std::vector<float> frequencyWindowMagnitudes = audioProcessor.getFrequencyWindowMagnitudes();
        modifyLogAlternation(frequencyWindowMagnitudes);

        if (!frequencyWindowMagnitudes.empty())
        {
            transparentWindow.setBarHeights(frequencyWindowMagnitudes);
        }

        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 0.7f); // Set background color with transparency
        glClear(GL_COLOR_BUFFER_BIT);

        transparentWindow.draw();

        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(displayIntervalMs));
    }

    audioProcessor.stopProcessing();
    audioCapture.stopCapture();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}