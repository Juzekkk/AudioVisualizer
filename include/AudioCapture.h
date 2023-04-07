#pragma once

#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>

class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();

    bool initialize();
    void startCapture();
    void stopCapture();
    size_t getBufferSize() const;
    float getSampleRate() const;
    std::vector<float> getOutputBuffer();
    bool hasNewData() const;
    std::condition_variable newDataAvailable;

private:
    static DWORD WINAPI captureThread(LPVOID lpParameter);
    void processAudio();

    HRESULT initializeCOM();
    HRESULT initializeAudioCapture();

    void releaseResources();
    void releaseInterface(IUnknown *pInterface);

    IMMDeviceEnumerator *pEnumerator;
    IMMDevice *pDevice;
    IAudioClient *pAudioClient;
    IAudioCaptureClient *pCaptureClient;

    WAVEFORMATEX *pwfx;
    HANDLE captureThreadHandle;
    DWORD captureThreadId;
    bool isCapturing;
    std::vector<float> audioData;
    std::mutex audioDataMutex;
    size_t bufferSize;
    std::mutex outputBufferMutex;
    std::vector<float> outputBuffer;
    bool newData;
};