#include "AudioProcessor.h"
#include "FFT.h"
#include <cmath>
#include <complex>
#include <algorithm>

AudioProcessor::AudioProcessor(unsigned int numFrequencyWindows, AudioCapture &audioCapture)
    : audioCapture(audioCapture),
      numFrequencyWindows(numFrequencyWindows),
      isProcessing(false),
      processingThreadHandle(nullptr),
      processingThreadId(0),
      audioDataMutex{}
{
}

AudioProcessor::~AudioProcessor()
{
    stopProcessing();
}

void AudioProcessor::startProcessing()
{
    if (isProcessing)
    {
        return;
    }

    isProcessing = true;
    processingThreadHandle = CreateThread(NULL, 0, processingThreadEntryPoint, this, 0, &processingThreadId);
}

void AudioProcessor::stopProcessing()
{
    if (!isProcessing)
    {
        return;
    }

    isProcessing = false;
    WaitForSingleObject(processingThreadHandle, INFINITE);
    CloseHandle(processingThreadHandle);
}

std::vector<float> AudioProcessor::getFrequencyWindowMagnitudes()
{
    std::unique_lock<std::mutex> lock(frequencyWindowMagnitudesMutex);
    return frequencyWindowMagnitudes;
}

DWORD WINAPI AudioProcessor::processingThreadEntryPoint(LPVOID lpParameter)
{
    AudioProcessor *audioProcessor = static_cast<AudioProcessor *>(lpParameter);
    audioProcessor->processAudio();
    return 0;
}

void AudioProcessor::processAudio()
{
    const double lowerFrequency = 20.0;
    const double upperFrequency = 4000.0;

    while (isProcessing)
    {
        std::vector<float> audioData(0);
        {
            std::unique_lock<std::mutex> lock(audioDataMutex);
            audioCapture.newDataAvailable.wait(lock, [&]()
                                               { return audioCapture.hasNewData(); });
            audioData = audioCapture.getOutputBuffer();
        }

        size_t bufferSize = audioData.size();

        if (bufferSize == 1024)
        {
            frequencyWindowMagnitudes = calculateFrequencyWindowMagnitudes(audioData, lowerFrequency, upperFrequency);
        }
    }
}

std::vector<float> AudioProcessor::calculateFrequencyWindowMagnitudes(const std::vector<float> &audioData, double lowerFrequency, double upperFrequency)
{
    std::vector<std::complex<double>> samples(audioData.begin(), audioData.begin() + audioData.size());

    // Apply the FFT
    std::vector<std::complex<double>> fftResult = fft(samples);

    // Calculate magnitudes
    std::vector<double> magnitudes;
    for (const std::complex<double> &value : fftResult)
    {
        magnitudes.push_back(std::abs(value));
    }

    // Calculate the frequency window magnitudes
    std::vector<float> tempFrequencyWindowMagnitudes(numFrequencyWindows, 0);
    double binWidth = audioCapture.getSampleRate() / static_cast<double>(audioData.size());
    double windowSizeFactor = (upperFrequency - lowerFrequency) / numFrequencyWindows;

    for (unsigned int i = 0; i < numFrequencyWindows; ++i)
    {
        double windowStartFrequency = lowerFrequency + i * windowSizeFactor;
        double windowEndFrequency = windowStartFrequency + windowSizeFactor;

        int binStart = static_cast<int>(std::ceil(windowStartFrequency / binWidth));
        int binEnd = static_cast<int>(std::floor(windowEndFrequency / binWidth));

        double sum = 0.0;
        for (int bin = binStart; bin <= binEnd; ++bin)
        {
            sum += magnitudes[bin];
        }
        tempFrequencyWindowMagnitudes[i] = static_cast<float>(sum / (binEnd - binStart + 1));
    }

    return tempFrequencyWindowMagnitudes;
}
