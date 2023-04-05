#pragma once

#include "AudioCapture.h"
#include <vector>
#include <mutex>
#include <windows.h>

class AudioProcessor
{
public:
    AudioProcessor(unsigned int numWindows, AudioCapture &audioCapture);
    ~AudioProcessor();

    void startProcessing();
    void stopProcessing();
    std::vector<float> getFrequencyWindowMagnitudes();

private:
    static DWORD WINAPI processingThread(LPVOID lpParameter);
    void processAudio();
    std::pair<double, double> getSection(double start, double end, int numOfParts, int sectionNumber);

    AudioCapture &audioCapture;
    unsigned int numWindows;
    bool isProcessing;
    HANDLE processingThreadHandle;
    DWORD processingThreadId;
    std::mutex frequencyWindowMagnitudesMutex;
    std::vector<float> frequencyWindowMagnitudes;
    size_t bufferSize;
};