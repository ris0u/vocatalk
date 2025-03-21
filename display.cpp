#include "display.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <sstream>

// SSD1306 OLED display commands
#define SSD1306_ADDR 0x3C
#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7

// Font data (5x8 font)
// This would be a large array of font data for all ASCII characters
// For brevity, I'm not including the full font data here

Display::Display(int width, int height) 
    : width(width), height(height), brightness(255), is_inverted(false), i2c_fd(-1) {
    if (!initializeI2C()) {
        throw std::runtime_error("Failed to initialize I2C for display");
    }
    
    // Initialize display
    sendCommand(SSD1306_DISPLAYOFF);
    
    // Various initialization commands for SSD1306
    // ... (omitted for brevity)
    
    // Set contrast
    sendCommand(SSD1306_SETCONTRAST);
    sendCommand(brightness);
    
    // Normal display (not inverted)
    sendCommand(SSD1306_NORMALDISPLAY);
    
    // Turn on display
    sendCommand(SSD1306_DISPLAYON);
    
    // Clear the display initially
    clear();
    update();
}

Display::~Display() {
    closeI2C();
}

bool Display::initializeI2C() {
    // Open I2C device
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        std::cerr << "Failed to open I2C device" << std::endl;
        return false;
    }
    
    // Set I2C slave address
    if (ioctl(i2c_fd, I2C_SLAVE, SSD1306_ADDR) < 0) {
        std::cerr << "Failed to set I2C slave address" << std::endl;
        close(i2c_fd);
        i2c_fd = -1;
        return false;
    }
    
    return true;
}

void Display::closeI2C() {
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
}

void Display::sendCommand(uint8_t command) {
    uint8_t buffer[2] = {SSD1306_COMMAND, command};
    if (write(i2c_fd, buffer, 2) != 2) {
        std::cerr << "Error writing command to display" << std::endl;
    }
}

void Display::sendData(uint8_t data) {
    uint8_t buffer[2] = {SSD1306_DATA, data};
    if (write(i2c_fd, buffer, 2) != 2) {
        std::cerr << "Error writing data to display" << std::endl;
    }
}

void Display::clear() {
    // Implementation would clear the display buffer
    // For brevity, not showing the full implementation
}

void Display::showText(const std::string& text) {
    std::vector<std::string> lines;
    wrapText(text, lines);
    showMultilineText(lines);
}

void Display::wrapText(const std::string& text, std::vector<std::string>& lines) {
    // Simple text wrapping implementation
    // For a 128x64 display with 5x8 font, we can fit about 21 characters per line
    // and 8 lines total
    
    const int chars_per_line = 21;
    std::istringstream iss(text);
    std::string word;
    std::string current_line;
    
    while (iss >> word) {
        if (current_line.empty()) {
            current_line = word;
        } else if (current_line.length() + word.length() + 1 <= chars_per_line) {
            current_line += " " + word;
        } else {
            lines.push_back(current_line);
            current_line = word;
        }
    }
    
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    
    // Limit to 8 lines
    if (lines.size() > 8) {
        lines.resize(8);
    }
}

void Display::showMultilineText(const std::vector<std::string>& lines) {
    // Implementation would render text lines to the display buffer
    // For brevity, not showing the full implementation
}

void Display::drawProgressBar(float percentage) {
    // Implementation would draw a progress bar on the display
    // For brevity, not showing the full implementation
}

void Display::update() {
    // Implementation would send the display buffer to the OLED
    // For brevity, not showing the full implementation
}

void Display::setBrightness(int new_brightness) {
    brightness = new_brightness;
    sendCommand(SSD1306_SETCONTRAST);
    sendCommand(brightness);
}

void Display::setInvertDisplay(bool invert) {
    is_inverted = invert;
    sendCommand(invert ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
} 