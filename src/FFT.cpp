#include "FFT.h"
#include <cmath>
#include <vector>
#include <complex>
#include <algorithm>
#include <bitset>

std::vector<std::complex<double>> fft(const std::vector<std::complex<double>> &samples)
{
    size_t n = samples.size();
    size_t log2n = std::log2(n);

    if (!std::bitset<32>(n).count() == 1)
        return std::vector<std::complex<double>>(); // Check if n is a power of 2

    // Rearrange the input samples using bit-reversal
    std::vector<std::complex<double>> samplesCopy(n);
    for (size_t i = 0; i < n; i++)
    {
        size_t reversed = 0;
        for (size_t b = 0; b < log2n; b++)
            reversed |= (i >> b & 1) << (log2n - 1 - b);
        samplesCopy[reversed] = samples[i];
    }

    // Apply the Cooley-Tukey algorithm iteratively
    std::vector<std::complex<double>> result = samplesCopy;
    for (size_t s = 1; s <= log2n; s++)
    {
        size_t m = 1 << s;
        std::complex<double> wm(cos(-2 * M_PI / m), sin(-2 * M_PI / m));

        for (size_t k = 0; k < n; k += m)
        {
            std::complex<double> w(1, 0);
            for (size_t j = 0; j < m / 2; j++)
            {
                std::complex<double> t = w * result[k + j + m / 2];
                std::complex<double> u = result[k + j];
                result[k + j] = u + t;
                result[k + j + m / 2] = u - t;
                w *= wm;
            }
        }
    }

    return result;
}
