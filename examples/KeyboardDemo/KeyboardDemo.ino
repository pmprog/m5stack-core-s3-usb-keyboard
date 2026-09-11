#include <Arduino.h>
#include "USBKeyboardHandler.h"

// Display update variables
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // 100ms

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("M5Stack Core S3 SE - USB Keyboard");
    Serial.println("====================================\n");
    
    // Initialize the USB keyboard handler
    if (!USBKeyboardHandler::begin()) {
        Serial.println("Failed to initialize USB Keyboard Handler!");
        while (1) {
            delay(1000);
        }
    }
    
    // Register keyboard event callback
    USBKeyboardHandler::onKeyboardEvent([](const keyboard_event_t& event) {
        Serial.printf("[KEY] Modifier: 0x%02X, Code: 0x%02X (%s), ASCII: '%c', Pressed: %s\n",
                      event.modifier,
                      event.scan_code,
                      USBKeyboardHandler::scanCodeToName(event.scan_code),
                      event.ascii_char ? event.ascii_char : '?',
                      event.is_pressed ? "yes" : "no");
    });
    
    Serial.println("Waiting for USB keyboard connection...");
}

void loop() {
    // Update USB host tasks
    USBKeyboardHandler::update();
    
    // Periodic status update
    if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = millis();
        
        // Print connection status
        if (USBKeyboardHandler::isKeyboardConnected()) {
            Serial.println("Status: USB Keyboard CONNECTED");
        } else {
            Serial.println("Status: Waiting for keyboard...");
        }
    }
    
    delay(10);
}
