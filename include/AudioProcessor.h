#pragma once

#include "AudioCapture.h"
#include <vector>
#include <mutex>
#include <windows.h>

class AudioProcessor
{
public:
    AudioProcessor(unsigned int numFrequencyWindows, AudioCapture &audioCapture);
    ~AudioProcessor();

    void startProcessing();
    void stopProcessing();
    std::vector<float> getFrequencyWindowMagnitudes();

private:
    static DWORD WINAPI processingThreadEntryPoint(LPVOID lpParameter);
    void processAudio();
    std::vector<float> calculateFrequencyWindowMagnitudes(const std::vector<float> &audioData, double lowerFrequency, double upperFrequency);

    AudioCapture &audioCapture;
    unsigned int numFrequencyWindows;
    bool isProcessing;
    HANDLE processingThreadHandle;
    DWORD processingThreadId;
    std::mutex frequencyWindowMagnitudesMutex;
    std::vector<float> frequencyWindowMagnitudes;
    std::mutex audioDataMutex;
};
