#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "AudioCapture.h"
#include "AudioProcessor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdlib>

void clearConsole()
{
#ifdef _WIN32
    std::system("CLS");
#else
    std::system("clear");
#endif
}

void modifyLogAlternation(std::vector<float> &vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = std::log(vec[i] + 1) / std::log(100);
    }
}

int main()
{
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
    const int maxBarLength = 40;

    while (true)
    {
        std::vector<float> frequencyWindowMagnitudes = audioProcessor.getFrequencyWindowMagnitudes();
        modifyLogAlternation(frequencyWindowMagnitudes);
        if (!frequencyWindowMagnitudes.empty())
        {
            clearConsole();
            for (float magnitude : frequencyWindowMagnitudes)
            {
                int barLength = static_cast<int>(std::round(magnitude * maxBarLength));
                for (int i = 0; i < barLength; ++i)
                {
                    std::cout << '#';
                }
                std::cout << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(displayIntervalMs));
    }

    audioProcessor.stopProcessing();
    audioCapture.stopCapture();

    return 0;
}
