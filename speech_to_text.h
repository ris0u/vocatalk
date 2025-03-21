#ifndef SPEECH_TO_TEXT_H
#define SPEECH_TO_TEXT_H

#include <string>
#include "audio_capture.h"

class SpeechToText {
public:
    enum Engine {
        WHISPER,
        VOSK,
        DEEPSPEECH
    };
    
    SpeechToText(const std::string& engine_name = "whisper");
    ~SpeechToText();
    
    std::string transcribe(const AudioBuffer& audio);
    void setEngine(const std::string& engine_name);
    void setLanguage(const std::string& language_code);
    
private:
    Engine engine;
    std::string language;
    void* engine_handle;  // Opaque pointer to engine-specific data
    
    bool initializeEngine();
    void cleanupEngine();
    
    // Engine-specific transcription functions
    std::string transcribeWithWhisper(const AudioBuffer& audio);
    std::string transcribeWithVosk(const AudioBuffer& audio);
    std::string transcribeWithDeepSpeech(const AudioBuffer& audio);
};

#endif // SPEECH_TO_TEXT_H 