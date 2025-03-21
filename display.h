#ifndef DISPLAY_H
#define DISPLAY_H

#include <string>
#include <vector>

class Display {
public:
    Display(int width, int height);
    ~Display();
    
    void clear();
    void showText(const std::string& text);
    void showMultilineText(const std::vector<std::string>& lines);
    void drawProgressBar(float percentage);
    void update();
    
    void setBrightness(int brightness);
    void setInvertDisplay(bool invert);
    
private:
    int width;
    int height;
    int brightness;
    bool is_inverted;
    int i2c_fd;
    
    bool initializeI2C();
    void closeI2C();
    void sendCommand(uint8_t command);
    void sendData(uint8_t data);
    void wrapText(const std::string& text, std::vector<std::string>& lines);
};

#endif // DISPLAY_H 