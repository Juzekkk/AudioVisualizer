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
    bool isReady() const;
    void waitUntilReady();

private:
    static DWORD WINAPI processingThreadEntryPoint(LPVOID lpParameter);
    void processAudio();
    std::vector<float> calculateFrequencyWindowMagnitudes(const std::vector<float> &audioData, double lowerFrequency, double upperFrequency) const;
    void modifyLogAlternation(std::vector<float> &vec);

    AudioCapture &audioCapture;
    unsigned int numFrequencyWindows;
    bool isProcessing;
    HANDLE processingThreadHandle;
    DWORD processingThreadId;
    std::mutex frequencyWindowMagnitudesMutex;
    std::vector<float> frequencyWindowMagnitudes;
    std::mutex audioDataMutex;
    bool packageReady;
    std::mutex readyMutex;
    std::condition_variable cv;
};
