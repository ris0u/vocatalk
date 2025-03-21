#ifndef HAPTIC_H
#define HAPTIC_H

#include <chrono>

class HapticFeedback {
public:
    HapticFeedback(int gpio_pin = 18);  // Default to GPIO 18
    ~HapticFeedback();
    
    void triggerVibration(int duration_ms = 200);
    void setIntensity(float intensity);  // 0.0 to 1.0
    void setPulsePattern(const std::string& pattern);
    
private:
    int gpio_pin;
    float intensity;
    std::string pulse_pattern;
    bool is_vibrating;
    
    bool initializeGPIO();
    void cleanupGPIO();
    void setGPIOValue(int value);
};

#endif // HAPTIC_H 