#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <functional>
#include <iostream>

class AudioCapture
{
private:
    IMMDeviceEnumerator *deviceEnumerator;
    IMMDevice *audioDevice;
    IAudioClient *audioClient;
    WAVEFORMATEX *format;
    IAudioCaptureClient *captureClient;
    UINT32 bufferFrameCount;
    std::function<void(BYTE *, LONG)> processDataCallback;

public:
    AudioCapture();
    ~AudioCapture();
    void initialize();
    void startCapture();
    void stopCapture();
    void setCallback(const std::function<void(BYTE *, LONG)> &callback);
    const WAVEFORMATEX *getFormat() const;
};
