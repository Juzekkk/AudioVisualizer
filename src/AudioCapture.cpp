#include "AudioCapture.h"
#include <stdexcept>

AudioCapture::AudioCapture() : pEnumerator(nullptr),
                               pDevice(nullptr),
                               pAudioClient(nullptr),
                               pCaptureClient(nullptr),
                               pwfx(nullptr),
                               captureThreadHandle(nullptr),
                               captureThreadId(0),
                               isCapturing(false),
                               bufferSize(0)
{
}

AudioCapture::~AudioCapture()
{
    stopCapture();

    if (pCaptureClient)
        pCaptureClient->Release();

    if (pAudioClient)
        pAudioClient->Release();

    if (pDevice)
        pDevice->Release();

    if (pEnumerator)
        pEnumerator->Release();

    if (pwfx)
        CoTaskMemFree(pwfx);
}

bool AudioCapture::initialize()
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
        return false;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);
    if (FAILED(hr))
        return false;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr))
        return false;

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&pAudioClient);
    if (FAILED(hr))
        return false;

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr))
        return false;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pwfx, nullptr);
    if (FAILED(hr))
        return false;

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void **)&pCaptureClient);
    if (FAILED(hr))
        return false;

    return true;
}

void AudioCapture::startCapture()
{
    if (isCapturing)
        return;

    isCapturing = true;
    HRESULT hr = pAudioClient->Start();
    if (FAILED(hr))
        throw std::runtime_error("Failed to start audio client");

    captureThreadHandle = CreateThread(nullptr, 0, captureThread, this, 0, &captureThreadId);
}

void AudioCapture::stopCapture()
{
    if (!isCapturing)
        return;

    isCapturing = false;
    pAudioClient->Stop();

    WaitForSingleObject(captureThreadHandle, INFINITE);
    CloseHandle(captureThreadHandle);
    captureThreadHandle = nullptr;
    captureThreadId = 0;
}

size_t AudioCapture::getBufferSize() const
{
    return bufferSize;
}

float AudioCapture::getSampleRate() const
{
    return static_cast<float>(pwfx->nSamplesPerSec);
}

DWORD WINAPI AudioCapture::captureThread(LPVOID lpParameter)
{
    AudioCapture *pThis = static_cast<AudioCapture *>(lpParameter);
    pThis->processAudio();
    return 0;
}

void AudioCapture::processAudio()
{
    while (isCapturing)
    {
        UINT32 numFramesAvailable;
        BYTE *pData;
        DWORD flags;

        HRESULT hr = pCaptureClient->GetNextPacketSize(&numFramesAvailable);
        if (FAILED(hr))
            break;

        while (numFramesAvailable > 0)
        {
            UINT32 numFramesToRead;
            hr = pCaptureClient->GetBuffer(&pData, &numFramesToRead, &flags, nullptr, nullptr);
            if (FAILED(hr))
                break;

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = nullptr;
            }

            size_t bufferLength = numFramesToRead * pwfx->nBlockAlign;
            bufferSize = bufferLength / sizeof(float);

            std::vector<float> tempData(bufferSize);
            std::copy(pData, pData + bufferLength, reinterpret_cast<BYTE *>(tempData.data()));

            {
                std::unique_lock<std::mutex> lock(outputBufferMutex);
                outputBuffer.swap(tempData);
                newData = true; // Set the newData flag
            }
            newDataAvailable.notify_one();

            hr = pCaptureClient->ReleaseBuffer(numFramesToRead);
            if (FAILED(hr))
                break;

            hr = pCaptureClient->GetNextPacketSize(&numFramesAvailable);
            if (FAILED(hr))
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool AudioCapture::hasNewData() const
{
    return newData;
}

std::vector<float> AudioCapture::getOutputBuffer()
{
    std::unique_lock<std::mutex> lock(outputBufferMutex);
    newData = false;
    return outputBuffer;
}