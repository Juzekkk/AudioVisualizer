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

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    TransparentWindow transparentWindow;
    transparentWindow.waitUntilTransparentWindowIsRunning();

    while (transparentWindow.isRunning())
    {
        audioProcessor.waitUntilReady();
        std::vector<float> frequencyWindowMagnitudes = audioProcessor.getFrequencyWindowMagnitudes();
        if (!frequencyWindowMagnitudes.empty())
        {
            transparentWindow.setBarHeights(frequencyWindowMagnitudes);
        }
    }

    transparentWindow.waitForClose();
    glfwTerminate();

    std::cout << "Window closed successfully. Exiting..." << std::endl;

    return 0;
}
