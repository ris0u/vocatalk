#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <signal.h>

// Hardware interfaces
#include "audio_capture.h"
#include "display.h"
#include "haptic.h"
#include "power_manager.h"
#include "bluetooth_manager.h"
#include "wifi_manager.h"

// Processing modules
#include "speech_to_text.h"
#include "noise_reduction.h"
#include "keyword_detector.h"
#include "storage_manager.h"

// Global control flags
std::atomic<bool> g_running(true);
std::atomic<bool> g_low_power_mode(false);
std::mutex g_text_mutex;
std::string g_current_transcription;
std::vector<std::string> g_transcription_history;

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    g_running = false;
}

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Audio processing thread function
void audioProcessingThread(AudioCapture& audio, SpeechToText& stt, 
                          NoiseReduction& noise, KeywordDetector& keyword,
                          HapticFeedback& haptic) {
    while (g_running) {
        // Capture audio
        AudioBuffer buffer = audio.captureAudio();
        
        // Apply noise reduction
        buffer = noise.processAudio(buffer);
        
        // Convert speech to text
        std::string text = stt.transcribe(buffer);
        
        if (!text.empty()) {
            // Check for keywords
            if (keyword.detectKeywords(text)) {
                haptic.triggerVibration();
            }
            
            // Update current transcription
            {
                std::lock_guard<std::mutex> lock(g_text_mutex);
                g_current_transcription = text;
                g_transcription_history.push_back(text);
                
                // Limit history size
                if (g_transcription_history.size() > 100) {
                    g_transcription_history.erase(g_transcription_history.begin());
                }
            }
        }
        
        // Sleep briefly to prevent CPU overuse
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Display update thread function
void displayUpdateThread(Display& display) {
    while (g_running) {
        std::string text_to_display;
        {
            std::lock_guard<std::mutex> lock(g_text_mutex);
            text_to_display = g_current_transcription;
        }
        
        display.clear();
        display.showText(text_to_display);
        display.update();
        
        // Update display at reasonable rate
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Storage thread function
void storageThread(StorageManager& storage) {
    std::vector<std::string> last_saved_transcriptions;
    
    while (g_running) {
        std::vector<std::string> current_transcriptions;
        {
            std::lock_guard<std::mutex> lock(g_text_mutex);
            current_transcriptions = g_transcription_history;
        }
        
        // Only save new transcriptions
        if (current_transcriptions.size() > last_saved_transcriptions.size()) {
            for (size_t i = last_saved_transcriptions.size(); i < current_transcriptions.size(); i++) {
                storage.saveTranscription(getCurrentTimestamp(), current_transcriptions[i]);
            }
            last_saved_transcriptions = current_transcriptions;
        }
        
        // Check less frequently to save power
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

// Power management thread
void powerManagementThread(PowerManager& power) {
    while (g_running) {
        float battery_level = power.getBatteryLevel();
        
        // Switch to low power mode if battery is below threshold
        if (battery_level < 0.2) {  // 20%
            g_low_power_mode = true;
        } else if (battery_level > 0.3) {  // 30%
            g_low_power_mode = false;
        }
        
        power.updatePowerMode(g_low_power_mode);
        
        // Check battery less frequently
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

// Connectivity thread (Bluetooth & WiFi)
void connectivityThread(BluetoothManager& bt, WiFiManager& wifi, StorageManager& storage) {
    while (g_running) {
        // Handle Bluetooth connections and data sync
        if (bt.isConnected()) {
            std::vector<std::string> transcriptions;
            {
                std::lock_guard<std::mutex> lock(g_text_mutex);
                transcriptions = g_transcription_history;
            }
            bt.syncTranscriptions(transcriptions);
        }
        
        // Handle WiFi backup if enabled and connected
        if (wifi.isEnabled() && wifi.isConnected()) {
            wifi.backupTranscriptions(storage.getUnsyncedTranscriptions());
            storage.markTranscriptionsAsSynced();
        }
        
        // Check connectivity less frequently to save power
        std::this_thread::sleep_for(std::chrono::seconds(g_low_power_mode ? 300 : 60));
    }
}

int main() {
    // Register signal handler
    signal(SIGINT, signalHandler);
    
    std::cout << "Initializing wearable transcription system..." << std::endl;
    
    try {
        // Initialize hardware components
        AudioCapture audio(44100, 1);  // 44.1kHz, mono
        Display display(128, 64);      // 128x64 OLED
        HapticFeedback haptic;
        PowerManager power;
        BluetoothManager bluetooth;
        WiFiManager wifi;
        
        // Initialize processing modules
        NoiseReduction noise;
        SpeechToText stt("whisper");  // Using OpenAI Whisper
        KeywordDetector keyword({"emergency", "help", "alert"});  // Example keywords
        StorageManager storage("/home/pi/transcriptions");
        
        std::cout << "System initialized. Starting processing threads..." << std::endl;
        
        // Start processing threads
        std::thread audio_thread(audioProcessingThread, 
                                std::ref(audio), std::ref(stt), 
                                std::ref(noise), std::ref(keyword),
                                std::ref(haptic));
        
        std::thread display_thread(displayUpdateThread, std::ref(display));
        std::thread storage_thread(storageThread, std::ref(storage));
        std::thread power_thread(powerManagementThread, std::ref(power));
        std::thread connectivity_thread(connectivityThread, 
                                      std::ref(bluetooth), std::ref(wifi), 
                                      std::ref(storage));
        
        std::cout << "System running. Press Ctrl+C to exit." << std::endl;
        
        // Wait for threads to complete
        audio_thread.join();
        display_thread.join();
        storage_thread.join();
        power_thread.join();
        connectivity_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "System shutdown complete." << std::endl;
    return 0;
} 