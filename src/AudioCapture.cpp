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
                               bufferSize(0),
                               newData(false)
{
}

AudioCapture::~AudioCapture()
{
    stopCapture();
    releaseResources();
}

bool AudioCapture::initialize()
{
    if (FAILED(initializeCOM()))
        return false;

    if (FAILED(initializeAudioCapture()))
        return false;

    return true;
}

HRESULT AudioCapture::initializeCOM()
{
    return CoInitialize(nullptr);
}

HRESULT AudioCapture::initializeAudioCapture()
{
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);
    if (FAILED(hr))
        return hr;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr))
        return hr;

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&pAudioClient);
    if (FAILED(hr))
        return hr;

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr))
        return hr;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pwfx, nullptr);
    if (FAILED(hr))
        return hr;

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void **)&pCaptureClient);

    return hr;
}

void AudioCapture::releaseResources()
{
    releaseInterface(pCaptureClient);
    releaseInterface(pAudioClient);
    releaseInterface(pDevice);
    releaseInterface(pEnumerator);

    if (pwfx)
        CoTaskMemFree(pwfx);
}

void AudioCapture::releaseInterface(IUnknown *pInterface)
{
    if (pInterface)
        pInterface->Release();
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
            if (pData != nullptr)
            {
                std::copy(pData, pData + bufferLength, reinterpret_cast<BYTE *>(tempData.data()));

                {
                    std::unique_lock<std::mutex> lock(outputBufferMutex);
                    outputBuffer.swap(tempData);
                    newData = true; // Set the newData flag
                }
                newDataAvailable.notify_one();
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesToRead);
            if (FAILED(hr))
                break;

            hr = pCaptureClient->GetNextPacketSize(&numFramesAvailable);
            if (FAILED(hr))
                break;
        }
    }
}

bool AudioCapture::hasNewData() const
{
    return newData;
}

void AudioCapture::waitUntilNewDataAvailable()
{
    std::unique_lock<std::mutex> lock(outputBufferMutex);
    newDataAvailable.wait(lock, [this]()
                          { return this->hasNewData(); });
}

std::vector<float> AudioCapture::getOutputBuffer()
{
    std::unique_lock<std::mutex> lock(outputBufferMutex);
    newData = false;
    return outputBuffer;
}