#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "AudioCapture.h"
#include "AudioProcessor.h"
#include "TransparentWindow.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>

int main()
{
    AudioCapture audioCapture;

    if (!audioCapture.initialize())
    {
        std::cerr << "Failed to initialize AudioCapture" << std::endl;
        return 1;
    }

    audioCapture.startCapture();

    unsigned int numberOfWindows = 12;
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
        // Get frequency window magnitudes
        std::vector<float> frequencyWindowMagnitudes = audioProcessor.getFrequencyWindowMagnitudes();

        if (!frequencyWindowMagnitudes.empty())
        {
            transparentWindow.setBarHeights(frequencyWindowMagnitudes);
        }

        // Poll events and prepare for drawing
        glfwPollEvents();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the transparent window and swap buffers
        transparentWindow.draw();
        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(displayIntervalMs));
    }

    // Cleanup
    audioProcessor.stopProcessing();
    audioCapture.stopCapture();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
