#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <vector>
#include <cstdint>
#include <alsa/asoundlib.h>

// Simple audio buffer class
class AudioBuffer {
public:
    AudioBuffer() {}
    AudioBuffer(const std::vector<int16_t>& data) : samples(data) {}
    
    std::vector<int16_t> samples;
    size_t sampleRate = 44100;
    size_t channels = 1;
};

class AudioCapture {
public:
    AudioCapture(int sample_rate = 44100, int channels = 1);
    ~AudioCapture();
    
    AudioBuffer captureAudio(int duration_ms = 1000);
    void setGain(float gain);
    void setSampleRate(int sample_rate);
    
private:
    snd_pcm_t *capture_handle;
    int sample_rate;
    int channels;
    float gain;
    
    bool initializeALSA();
    void closeALSA();
};

#endif // AUDIO_CAPTURE_H 