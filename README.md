# M5Stack Core S3 SE USB Keyboard Handler

A comprehensive C++ driver for handling USB keyboards connected to the M5Stack Core S3 SE via USB-C OTG (On-The-Go).

## Features

- **HID Keyboard Support**: Full HID protocol implementation for standard USB keyboards
- **Event-Driven Architecture**: Callback-based system for keyboard events
- **ASCII Conversion**: Automatic conversion of HID scan codes to ASCII characters
- **Modifier Key Support**: Full support for Shift, Ctrl, Alt, and GUI keys
- **Connection Detection**: Monitor keyboard connection status
- **Debug Support**: Scan code name lookup for debugging

## Hardware Requirements

- M5Stack Core S3 SE
- USB Keyboard
- USB-A to USB-C OTG Dongle/Adapter

## Software Requirements

- Arduino IDE or PlatformIO
- ESP-IDF (v4.4 or later)
- TinyUSB library

## Installation

### Using PlatformIO

1. Clone this repository:
```bash
git clone https://github.com/pmprog/m5stack-core-s3-usb-keyboard.git
cd m5stack-core-s3-usb-keyboard
```

2. Build and upload:
```bash
platformio run -t upload
```

### Using Arduino IDE

1. Download the repository as ZIP
2. Add the library via Sketch → Include Library → Add .ZIP Library
3. Select the M5Stack Core S3 SE as your board
4. Open `examples/KeyboardDemo/KeyboardDemo.ino`
5. Upload to your device

## Quick Start

```cpp
#include "USBKeyboardHandler.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize the keyboard handler
    USBKeyboardHandler::begin();
    
    // Register a callback for keyboard events
    USBKeyboardHandler::onKeyboardEvent([](const keyboard_event_t& event) {
        Serial.printf("Key pressed: %c\n", event.ascii_char);
    });
}

void loop() {
    // Must be called regularly to process USB events
    USBKeyboardHandler::update();
    delay(10);
}
```

## API Reference

### Initialization

```cpp
bool USBKeyboardHandler::begin();
void USBKeyboardHandler::end();
```

Initialize and deinitialize the USB keyboard handler.

### Event Handling

```cpp
void USBKeyboardHandler::onKeyboardEvent(KeyboardCallback callback);
```

Register a callback function to receive keyboard events. The callback receives a `keyboard_event_t` structure containing:
- `modifier`: Modifier key state (Shift, Ctrl, Alt, GUI)
- `scan_code`: HID scan code of the pressed key
- `ascii_char`: ASCII representation (if applicable)
- `is_pressed`: True if key pressed, false if released

### Status

```cpp
bool USBKeyboardHandler::isKeyboardConnected();
```

Check if a keyboard is currently connected.

### Utilities

```cpp
char USBKeyboardHandler::scanCodeToAscii(uint8_t scan_code, uint8_t modifier, bool shift_pressed);
const char* USBKeyboardHandler::scanCodeToName(uint8_t scan_code);
```

Convert HID scan codes to ASCII characters or get human-readable names.

### Main Loop

```cpp
void USBKeyboardHandler::update();
```

Must be called regularly (at least every 100ms) to process USB host tasks.

## HID Scan Codes

The following scan codes are predefined:

### Letters
- `HID_KEY_A` through `HID_KEY_Z` (0x04-0x1D)

### Numbers
- `HID_KEY_0` through `HID_KEY_9` (0x27, 0x1E-0x26)

### Special Keys
- `HID_KEY_ENTER` (0x28)
- `HID_KEY_ESCAPE` (0x29)
- `HID_KEY_BACKSPACE` (0x2A)
- `HID_KEY_TAB` (0x2B)
- `HID_KEY_SPACE` (0x2C)

### Arrow Keys
- `HID_KEY_UP` (0x52)
- `HID_KEY_DOWN` (0x51)
- `HID_KEY_LEFT` (0x50)
- `HID_KEY_RIGHT` (0x4F)

### Function Keys
- `HID_KEY_F1` through `HID_KEY_F12` (0x3A-0x45)

## Modifier Keys

- `HID_MOD_LCTRL` - Left Control
- `HID_MOD_LSHIFT` - Left Shift
- `HID_MOD_LALT` - Left Alt
- `HID_MOD_LGUI` - Left GUI (Windows/Command)
- `HID_MOD_RCTRL` - Right Control
- `HID_MOD_RSHIFT` - Right Shift
- `HID_MOD_RALT` - Right Alt
- `HID_MOD_RGUI` - Right GUI

## Example Usage

### Basic Keyboard Logger

```cpp
#include "USBKeyboardHandler.h"

void onKeyPress(const keyboard_event_t& event) {
    if (event.is_pressed && event.ascii_char) {
        Serial.write(event.ascii_char);
    }
}

void setup() {
    Serial.begin(115200);
    USBKeyboardHandler::begin();
    USBKeyboardHandler::onKeyboardEvent(onKeyPress);
}

void loop() {
    USBKeyboardHandler::update();
    delay(10);
}
```

### Modifier Key Detection

```cpp
void onKeyPress(const keyboard_event_t& event) {
    if (event.modifier & HID_MOD_LCTRL) {
        Serial.println("Ctrl held");
    }
    if (event.modifier & HID_MOD_LSHIFT) {
        Serial.println("Shift held");
    }
}
```

## Troubleshooting

### Keyboard Not Detected

1. Verify the USB-C OTG adapter is functioning correctly
2. Check that your keyboard is HID-compliant
3. Look for debug messages in the Serial monitor
4. Try a different USB keyboard to rule out hardware issues

### Characters Not Mapping Correctly

1. Verify the keyboard layout matches your system
2. Check the modifier key states in the debug output
3. The ASCII table may need adjustment for non-US keyboard layouts

### Performance Issues

1. Ensure `USBKeyboardHandler::update()` is called frequently
2. Reduce other CPU-intensive tasks in your main loop
3. Increase the task priority if using FreeRTOS

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

MIT License - See LICENSE file for details

## References

- [M5Stack Core S3 SE Documentation](https://docs.m5stack.com/)
- [USB HID Specification](https://www.usb.org/hid)
- [TinyUSB Documentation](https://github.com/hathach/tinyusb)
- [ESP32-S3 USB Host Support](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/usb_host.html)

## Support

For issues and questions, please open a GitHub issue in this repository.
