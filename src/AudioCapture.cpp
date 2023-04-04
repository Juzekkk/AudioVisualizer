#include "AudioCapture.h"

AudioCapture::AudioCapture()
{
    deviceEnumerator = nullptr;
    audioDevice = nullptr;
    audioClient = nullptr;
    format = nullptr;
    captureClient = nullptr;
}

AudioCapture::~AudioCapture()
{
    if (captureClient)
        captureClient->Release();
    if (format)
        CoTaskMemFree(format);
    if (audioClient)
        audioClient->Release();
    if (audioDevice)
        audioDevice->Release();
    if (deviceEnumerator)
        deviceEnumerator->Release();
    CoUninitialize();
}

void AudioCapture::initialize()
{
    CoInitialize(NULL);

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&deviceEnumerator);

    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to create device enumerator." << std::endl;
        exit(EXIT_FAILURE);
    }

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audioDevice);
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to get default audio device." << std::endl;
        exit(EXIT_FAILURE);
    }

    hr = audioDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&audioClient);
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to activate audio client." << std::endl;
        exit(EXIT_FAILURE);
    }

    hr = audioClient->GetMixFormat(&format);
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to get mix format." << std::endl;
        exit(EXIT_FAILURE);
    }

    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, format, NULL);
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to initialize audio client." << std::endl;
        exit(EXIT_FAILURE);
    }

    hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void **)&captureClient);
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to get capture client service." << std::endl;
        exit(EXIT_FAILURE);
    }

    hr = audioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to get buffer size." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void AudioCapture::startCapture()
{
    HRESULT hr = audioClient->Start();
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to start audio client." << std::endl;
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        UINT32 numFramesAvailable;
        BYTE *pData;
        DWORD flags;

        hr = captureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        if (FAILED(hr))
        {
            std::cerr << "Error: Failed to get buffer." << std::endl;
            break;
        }

        if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
        {
            pData = NULL;
        }

        LONG bytesToWrite = numFramesAvailable * format->nBlockAlign;

        // Call the processDataCallback function
        if (processDataCallback)
        {
            processDataCallback(pData, bytesToWrite);
        }

        hr = captureClient->ReleaseBuffer(numFramesAvailable);
        if (FAILED(hr))
        {
            std::cerr << "Error: Failed to release buffer." << std::endl;
            break;
        }
    }

    stopCapture();
}

void AudioCapture::stopCapture()
{
    HRESULT hr = audioClient->Stop();
    if (FAILED(hr))
    {
        std::cerr << "Error: Failed to stop audio client." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void AudioCapture::setCallback(const std::function<void(BYTE *, LONG)> &callback)
{
    processDataCallback = callback;
}

const WAVEFORMATEX *AudioCapture::getFormat() const
{
    return format;
}