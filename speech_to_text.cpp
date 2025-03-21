#include "speech_to_text.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

// For Whisper implementation
#include "whisper.h"

SpeechToText::SpeechToText(const std::string& engine_name) 
    : language("en"), engine_handle(nullptr) {
    setEngine(engine_name);
    if (!initializeEngine()) {
        throw std::runtime_error("Failed to initialize speech-to-text engine");
    }
}

SpeechToText::~SpeechToText() {
    cleanupEngine();
}

void SpeechToText::setEngine(const std::string& engine_name) {
    std::string name_lower = engine_name;
    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                  [](unsigned char c){ return std::tolower(c); });
    
    if (name_lower == "whisper") {
        engine = WHISPER;
    } else if (name_lower == "vosk") {
        engine = VOSK;
    } else if (name_lower == "deepspeech") {
        engine = DEEPSPEECH;
    } else {
        std::cerr << "Unknown engine: " << engine_name << ". Defaulting to Whisper." << std::endl;
        engine = WHISPER;
    }
}

void SpeechToText::setLanguage(const std::string& language_code) {
    language = language_code;
}

bool SpeechToText::initializeEngine() {
    cleanupEngine();  // Clean up any existing engine
    
    switch (engine) {
        case WHISPER: {
            // Initialize Whisper
            struct whisper_context* ctx = whisper_init_from_file("/home/pi/models/ggml-tiny.en.bin");
            if (ctx == nullptr) {
                std::cerr << "Failed to initialize Whisper model" << std::endl;
                return false;
            }
            engine_handle = ctx;
            break;
        }
        case VOSK:
            // Initialize Vosk (implementation omitted for brevity)
            std::cout << "Vosk initialization would go here" << std::endl;
            break;
        case DEEPSPEECH:
            // Initialize DeepSpeech (implementation omitted for brevity)
            std::cout << "DeepSpeech initialization would go here" << std::endl;
            break;
    }
    
    return true;
}

void SpeechToText::cleanupEngine() {
    if (engine_handle != nullptr) {
        switch (engine) {
            case WHISPER:
                whisper_free((struct whisper_context*)engine_handle);
                break;
            case VOSK:
                // Cleanup Vosk
                break;
            case DEEPSPEECH:
                // Cleanup DeepSpeech
                break;
        }
        engine_handle = nullptr;
    }
}

std::string SpeechToText::transcribe(const AudioBuffer& audio) {
    switch (engine) {
        case WHISPER:
            return transcribeWithWhisper(audio);
        case VOSK:
            return transcribeWithVosk(audio);
        case DEEPSPEECH:
            return transcribeWithDeepSpeech(audio);
        default:
            return "Transcription engine not implemented";
    }
}

std::string SpeechToText::transcribeWithWhisper(const AudioBuffer& audio) {
    if (engine_handle == nullptr) {
        return "Whisper model not initialized";
    }
    
    struct whisper_context* ctx = (struct whisper_context*)engine_handle;
    
    // Convert int16_t samples to float
    std::vector<float> pcmf32(audio.samples.size());
    for (size_t i = 0; i < audio.samples.size(); i++) {
        pcmf32[i] = float(audio.samples[i]) / 32768.0f;
    }
    
    // Set up Whisper parameters
    struct whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime = false;
    params.print_progress = false;
    params.print_timestamps = false;
    params.translate = false;
    params.language = language.c_str();
    params.n_threads = 2;  // Use 2 threads on Raspberry Pi
    
    // Run inference
    if (whisper_full(ctx, params, pcmf32.data(), pcmf32.size()) != 0) {
        return "Failed to run Whisper inference";
    }
    
    // Get the result
    std::string result;
    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; i++) {
        result += whisper_full_get_segment_text(ctx, i);
        result += " ";
    }
    
    return result;
}

std::string SpeechToText::transcribeWithVosk(const AudioBuffer& audio) {
    // Vosk implementation would go here
    return "Vosk transcription not implemented";
}

std::string SpeechToText::transcribeWithDeepSpeech(const AudioBuffer& audio) {
    // DeepSpeech implementation would go here
    return "DeepSpeech transcription not implemented";
} 