#include <Arduino.h>
#include "USBKeyboardHandler.h"

// Buffer for storing typed text
static char textBuffer[256] = {0};
static int bufferIndex = 0;

// Command processing
void processCommand(const char* command) {
    Serial.print("Command: ");
    Serial.println(command);
    
    if (strcmp(command, "help") == 0) {
        Serial.println("Available commands:");
        Serial.println("  help    - Show this help message");
        Serial.println("  clear   - Clear the text buffer");
        Serial.println("  status  - Show keyboard status");
        Serial.println("  buffer  - Show current buffer contents");
    }
    else if (strcmp(command, "clear") == 0) {
        memset(textBuffer, 0, sizeof(textBuffer));
        bufferIndex = 0;
        Serial.println("Buffer cleared");
    }
    else if (strcmp(command, "status") == 0) {
        Serial.printf("Keyboard connected: %s\n", 
                      USBKeyboardHandler::isKeyboardConnected() ? "Yes" : "No");
        Serial.printf("Buffer size: %d/256\n", bufferIndex);
    }
    else if (strcmp(command, "buffer") == 0) {
        Serial.print("Buffer contents: ");
        Serial.println(textBuffer);
    }
    else {
        Serial.println("Unknown command. Type 'help' for available commands.");
    }
}

// Keyboard event handler
void onKeyboardEvent(const keyboard_event_t& event) {
    if (!event.is_pressed) return;  // Only process key presses
    
    // Handle special keys
    if (event.scan_code == HID_KEY_ENTER) {
        // Process command on Enter
        if (bufferIndex > 0) {
            textBuffer[bufferIndex] = '\0';
            processCommand(textBuffer);
            memset(textBuffer, 0, sizeof(textBuffer));
            bufferIndex = 0;
        }
        Serial.println();
        return;
    }
    
    if (event.scan_code == HID_KEY_BACKSPACE) {
        if (bufferIndex > 0) {
            bufferIndex--;
            textBuffer[bufferIndex] = '\0';
            Serial.write('\b');
            Serial.write(' ');
            Serial.write('\b');
        }
        return;
    }
    
    // Handle printable characters
    if (event.ascii_char && bufferIndex < 255) {
        textBuffer[bufferIndex++] = event.ascii_char;
        Serial.write(event.ascii_char);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("M5Stack Core S3 SE - USB Keyboard");
    Serial.println("Advanced Command Handler Example");
    Serial.println("====================================\n");
    
    // Initialize keyboard handler
    if (!USBKeyboardHandler::begin()) {
        Serial.println("ERROR: Failed to initialize USB Keyboard Handler!");
        while (1) delay(1000);
    }
    
    USBKeyboardHandler::onKeyboardEvent(onKeyboardEvent);
    
    Serial.println("Keyboard handler initialized.");
    Serial.println("Connect a USB keyboard and type commands.");
    Serial.println("Type 'help' for available commands.\n");
    Serial.print("> ");
}

void loop() {
    USBKeyboardHandler::update();
    delay(10);
}
