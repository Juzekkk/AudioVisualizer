#include "AudioProcessor.h"
#include "FFT.h"
#include <cmath>
#include <complex>
#include <algorithm>

AudioProcessor::AudioProcessor(unsigned int numWindows, AudioCapture &audioCapture)
    : audioCapture(audioCapture),
      numWindows(numWindows),
      isProcessing(false),
      processingThreadHandle(nullptr),
      processingThreadId(0),
      bufferSize(0)
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
    processingThreadHandle = CreateThread(NULL, 0, processingThread, this, 0, &processingThreadId);
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

DWORD WINAPI AudioProcessor::processingThread(LPVOID lpParameter)
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
        std::vector<float> audioData = audioCapture.getOutputBuffer();
        size_t bufferSize = audioData.size();

        if (bufferSize == 0)
        {
            Sleep(10);
            continue;
        }

        if (audioData.size() >= bufferSize)
        {
            std::vector<std::complex<double>> samples(audioData.begin(), audioData.begin() + bufferSize);

            // Apply the FFT
            std::vector<std::complex<double>> fftResult = fft(samples);

            // Calculate magnitudes
            std::vector<double> magnitudes;
            for (const std::complex<double> &value : fftResult)
            {
                magnitudes.push_back(std::abs(value));
            }

            // Calculate the frequency window magnitudes
            std::vector<float> tempFrequencyWindowMagnitudes(numWindows, 0);
            double binWidth = audioCapture.getSampleRate() / static_cast<double>(bufferSize);
            double windowSizeFactor = (upperFrequency - lowerFrequency) / numWindows;

            for (unsigned int i = 0; i < numWindows; ++i)
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

            {
                std::unique_lock<std::mutex> lock(frequencyWindowMagnitudesMutex);
                frequencyWindowMagnitudes = tempFrequencyWindowMagnitudes;
            }

            Sleep(10); // Sleep for 10 milliseconds to allow other threads to run
        }
    }
}
