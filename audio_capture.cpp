#include "audio_capture.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

AudioCapture::AudioCapture(int sample_rate, int channels) 
    : sample_rate(sample_rate), channels(channels), gain(1.0), capture_handle(nullptr) {
    if (!initializeALSA()) {
        throw std::runtime_error("Failed to initialize ALSA audio capture");
    }
}

AudioCapture::~AudioCapture() {
    closeALSA();
}

bool AudioCapture::initializeALSA() {
    int err;
    
    // Open PCM device for recording
    if ((err = snd_pcm_open(&capture_handle, "hw:1,0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        std::cerr << "Cannot open audio device: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Allocate hardware parameters object
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    
    // Fill it with default values
    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
        std::cerr << "Cannot configure this PCM device: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Set the desired hardware parameters
    
    // Interleaved mode
    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Signed 16-bit little-endian format
    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        std::cerr << "Cannot set sample format: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Set channels
    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels)) < 0) {
        std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Set sample rate
    unsigned int actual_rate = sample_rate;
    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &actual_rate, 0)) < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    if (actual_rate != (unsigned int)sample_rate) {
        std::cout << "Warning: actual sample rate (" << actual_rate 
                  << ") differs from requested rate (" << sample_rate << ")" << std::endl;
        sample_rate = actual_rate;
    }
    
    // Apply the hardware configuration
    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        std::cerr << "Cannot set parameters: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Prepare the PCM device for use
    if ((err = snd_pcm_prepare(capture_handle)) < 0) {
        std::cerr << "Cannot prepare audio interface: " << snd_strerror(err) << std::endl;
        return false;
    }
    
    return true;
}

void AudioCapture::closeALSA() {
    if (capture_handle) {
        snd_pcm_close(capture_handle);
        capture_handle = nullptr;
    }
}

AudioBuffer AudioCapture::captureAudio(int duration_ms) {
    int err;
    int frames_to_capture = (sample_rate * duration_ms) / 1000;
    std::vector<int16_t> buffer(frames_to_capture * channels);
    
    // Read the specified number of frames
    if ((err = snd_pcm_readi(capture_handle, buffer.data(), frames_to_capture)) != frames_to_capture) {
        if (err < 0) {
            std::cerr << "Error reading from PCM device: " << snd_strerror(err) << std::endl;
            // Try to recover
            snd_pcm_recover(capture_handle, err, 0);
        } else {
            std::cerr << "Warning: read " << err << " frames instead of " << frames_to_capture << std::endl;
        }
    }
    
    // Apply gain if needed
    if (gain != 1.0f) {
        for (auto& sample : buffer) {
            float value = sample * gain;
            // Clamp to int16_t range
            if (value > 32767.0f) value = 32767.0f;
            if (value < -32768.0f) value = -32768.0f;
            sample = static_cast<int16_t>(value);
        }
    }
    
    AudioBuffer result(buffer);
    result.sampleRate = sample_rate;
    result.channels = channels;
    return result;
}

void AudioCapture::setGain(float new_gain) {
    gain = new_gain;
}

void AudioCapture::setSampleRate(int new_rate) {
    if (new_rate != sample_rate) {
        sample_rate = new_rate;
        // Reinitialize ALSA with new sample rate
        closeALSA();
        if (!initializeALSA()) {
            throw std::runtime_error("Failed to reinitialize ALSA with new sample rate");
        }
    }
} 