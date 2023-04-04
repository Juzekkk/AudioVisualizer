#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <Windows.h>

class AudioProcessor
{
private:
    unsigned int numBars;
    std::vector<double> windowFunction;

    void applyWindowFunction(std::vector<std::complex<double>> &samples);
    std::vector<double> computeFFT(const std::vector<std::complex<double>> &samples);
    void fft(std::vector<std::complex<double>> &samples);
    std::vector<double> groupFrequencies(const std::vector<double> &magnitudes, int sampleRate);

public:
    AudioProcessor(unsigned int numBars);
    std::vector<double> process(BYTE *pData, LONG dataSize, const WAVEFORMATEX &format);
};