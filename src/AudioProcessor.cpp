#include "AudioProcessor.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

AudioProcessor::AudioProcessor(unsigned int numBars)
    : numBars(numBars)
{
    windowFunction.resize(numBars);
    for (unsigned int i = 0; i < numBars; ++i)
    {
        windowFunction[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (numBars - 1)));
    }
}

std::vector<double> AudioProcessor::process(BYTE *pData, LONG dataSize, const WAVEFORMATEX &format)
{
    int numChannels = format.nChannels;
    int bytesPerSample = format.wBitsPerSample / 8;

    int numSamples = dataSize / (bytesPerSample * numChannels);
    std::vector<std::complex<double>> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        int16_t sample = *reinterpret_cast<int16_t *>(pData + i * bytesPerSample * numChannels);
        samples[i] = static_cast<double>(sample) / 32767.0;
    }

    applyWindowFunction(samples);
    std::vector<double> magnitudes = computeFFT(samples);
    return groupFrequencies(magnitudes, format.nSamplesPerSec);
}

void AudioProcessor::applyWindowFunction(std::vector<std::complex<double>> &samples)
{
    for (unsigned int i = 0; i < samples.size(); ++i)
    {
        samples[i] *= windowFunction[i % numBars];
    }
}

std::vector<double> AudioProcessor::computeFFT(const std::vector<std::complex<double>> &samples)
{
    std::vector<std::complex<double>> fftSamples = samples;
    fft(fftSamples);

    std::vector<double> magnitudes(samples.size());
    for (size_t i = 0; i < samples.size(); ++i)
    {
        magnitudes[i] = std::abs(fftSamples[i]);
    }

    return magnitudes;
}

void AudioProcessor::fft(std::vector<std::complex<double>> &samples)
{
    int n = samples.size();
    if (n <= 1)
        return;

    std::vector<std::complex<double>> even(n / 2);
    std::vector<std::complex<double>> odd(n / 2);

    for (int i = 0; i < n / 2; ++i)
    {
        even[i] = samples[i * 2];
        odd[i] = samples[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (int i = 0; i < n / 2; ++i)
    {
        std::complex<double> t = std::polar(1.0, -2.0 * M_PI * i / n) * odd[i];
        samples[i] = even[i] + t;
        samples[i + n / 2] = even[i] - t;
    }
}

std::vector<double> AudioProcessor::groupFrequencies(const std::vector<double> &magnitudes, int sampleRate)
{
    std::vector<double> groupedMagnitudes(numBars, 0.0);
    int minFrequency = 20;    // 20 Hz - minimum audible frequency for humans
    int maxFrequency = 20000; // 20,000 Hz - maximum audible frequency for humans

    double minLog = std::log10(minFrequency);
    double maxLog = std::log10(maxFrequency);
    double logRange = maxLog - minLog;

    int numFrequencies = magnitudes.size();
    double freqPerBin = static_cast<double>(sampleRate) / (2 * numFrequencies);

    for (unsigned int i = 0; i < numBars; ++i)
    {
        double lowerFreq = std::pow(10, minLog + i * logRange / numBars);
        double upperFreq = std::pow(10, minLog + (i + 1) * logRange / numBars);

        int lowerIndex = static_cast<int>(std::ceil(lowerFreq / freqPerBin));
        int upperIndex = static_cast<int>(std::floor(upperFreq / freqPerBin));

        double sum = 0.0;
        int count = 0;

        for (int j = lowerIndex; j <= upperIndex && j < numFrequencies; ++j)
        {
            sum += magnitudes[j];
            count++;
        }

        if (count > 0)
        {
            groupedMagnitudes[i] = sum / count;
        }
    }

    return groupedMagnitudes;
}
