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
      audioDataMutex{},
      packageReady(false)
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
    processingThreadHandle = CreateThread(nullptr, 0, processingThreadEntryPoint, this, 0, &processingThreadId);
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
    packageReady = false;
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
    const double lowerFrequency = 40.0;
    const double upperFrequency = 20000.0;

    while (isProcessing)
    {
        std::vector<float> audioData(0);
        {
            std::unique_lock<std::mutex> lock(audioDataMutex);
            audioCapture.newDataAvailable.wait(lock, [&]()
                                               { return audioCapture.hasNewData(); });
            audioData = audioCapture.getOutputBuffer();
        }
        size_t nextPow2 = 1;
        while (nextPow2 < audioData.size())
        {
            nextPow2 <<= 1;
        }

        size_t bufferSize = audioData.size();
        if (nextPow2 == 1024)
        {
            std::unique_lock<std::mutex> lock(readyMutex);
            frequencyWindowMagnitudes = calculateFrequencyWindowMagnitudes(audioData, lowerFrequency, upperFrequency);
            modifyLogAlternation(frequencyWindowMagnitudes);
            packageReady = true;
        }
        cv.notify_one();
    }
}

std::vector<float> AudioProcessor::calculateFrequencyWindowMagnitudes(const std::vector<float> &audioData, double lowerFrequency, double upperFrequency) const
{
    std::vector<std::complex<double>> samples(audioData.begin(), audioData.begin() + audioData.size());
    samples.resize(1024, std::complex<double>(0.0, 0.0));

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

    double logLowerFrequency = std::log10(lowerFrequency);
    double logUpperFrequency = std::log10(upperFrequency);
    double logRange = logUpperFrequency - logLowerFrequency;

    double scalingFactor = 1.5; // Adjust this value to control the window size growth

    for (unsigned int i = 0; i < numFrequencyWindows; ++i)
    {
        double scaleFactor = 1.0 + (i * (scalingFactor - 1.0)) / (numFrequencyWindows - 1);

        double logWindowStartFrequency = logLowerFrequency + (i * logRange) / (numFrequencyWindows * scaleFactor);
        double logWindowEndFrequency = logLowerFrequency + ((i + 1) * logRange) / (numFrequencyWindows * scaleFactor);

        double windowStartFrequency = std::pow(10, logWindowStartFrequency);
        double windowEndFrequency = std::pow(10, logWindowEndFrequency);

        int binStart = static_cast<int>(std::ceil(windowStartFrequency / binWidth));
        int binEnd = static_cast<int>(std::floor(windowEndFrequency / binWidth));

        double sum = 0.0;
        for (int bin = binStart; bin <= binEnd; ++bin)
        {
            if (bin >= 0 && bin < magnitudes.size())
            {
                sum += magnitudes[bin];
            }
        }
        tempFrequencyWindowMagnitudes[i] = static_cast<float>(sum / (binEnd - binStart + 1));
    }

    return tempFrequencyWindowMagnitudes;
}

void AudioProcessor::modifyLogAlternation(std::vector<float> &vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = std::log(vec[i] + 1) / 8;
    }
}

bool AudioProcessor::isReady() const
{
    return packageReady;
}

void AudioProcessor::waitUntilReady()
{
    std::unique_lock<std::mutex> lock(readyMutex);
    cv.wait(lock, [this]()
            { return this->isReady(); });
}
