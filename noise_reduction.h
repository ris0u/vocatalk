#ifndef NOISE_REDUCTION_H
#define NOISE_REDUCTION_H

#include "audio_capture.h"

class NoiseReduction {
public:
    NoiseReduction();
    ~NoiseReduction();
    
    AudioBuffer processAudio(const AudioBuffer& input);
    void setNoiseProfile(const AudioBuffer& noise_profile);
    void setReductionLevel(float level);
    void enableAdaptiveMode(bool enable);
    
private:
    float reduction_level;
    bool adaptive_mode;
    AudioBuffer noise_profile;
    
    // FFT-related members
    void* fft_handle;
    int fft_size;
    
    bool initializeFFT();
    void cleanupFFT();
    
    // Spectral subtraction method
    AudioBuffer spectralSubtraction(const AudioBuffer& input);
    
    // Adaptive noise estimation
    void updateNoiseProfile(const AudioBuffer& input);
};

#endif // NOISE_REDUCTION_H 