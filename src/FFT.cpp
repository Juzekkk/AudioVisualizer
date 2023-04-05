#include "FFT.h"
#include <cmath>

std::vector<std::complex<double>> fft(const std::vector<std::complex<double>> &samples)
{
    size_t n = samples.size();

    if (n == 1)
    {
        return samples;
    }

    std::vector<std::complex<double>> evenSamples(n / 2);
    std::vector<std::complex<double>> oddSamples(n / 2);

    for (size_t i = 0; i < n / 2; ++i)
    {
        evenSamples[i] = samples[i * 2];
        oddSamples[i] = samples[i * 2 + 1];
    }

    std::vector<std::complex<double>> fftEven = fft(evenSamples);
    std::vector<std::complex<double>> fftOdd = fft(oddSamples);

    double angle = -2.0 * M_PI / static_cast<double>(n);
    std::complex<double> omega = std::complex<double>(cos(angle), sin(angle));
    std::complex<double> omegaCurrent(1, 0);

    std::vector<std::complex<double>> result(n);

    for (size_t i = 0; i < n / 2; ++i)
    {
        result[i] = fftEven[i] + omegaCurrent * fftOdd[i];
        result[i + n / 2] = fftEven[i] - omegaCurrent * fftOdd[i];
        omegaCurrent *= omega;
    }

    return result;
}
