#include "FFT.h"
#include <cmath>
#include <vector>
#include <complex>
#include <algorithm>

void bitReverse(std::vector<std::complex<double>> &data)
{
    size_t n = data.size();
    size_t log2n = std::log2(n);
    for (size_t i = 1, j = 0; i < n; ++i)
    {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
        {
            j ^= bit;
        }
        j ^= bit;
        if (i < j)
        {
            std::swap(data[i], data[j]);
        }
    }
}

std::vector<std::complex<double>> fft(const std::vector<std::complex<double>> &samples)
{
    size_t n = samples.size();
    size_t log2n = std::log2(n);

    if (n & (n - 1)) // Check if n is a power of 2
    {
        return std::vector<std::complex<double>>();
    }

    std::vector<std::complex<double>> result = samples;
    bitReverse(result);

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
