#include "USBKeyboardHandler.h"
#include <Arduino.h>
#include "tusb.h"

// Static member initialization
KeyboardCallback USBKeyboardHandler::_callback = nullptr;
bool USBKeyboardHandler::_keyboard_connected = false;
uint8_t USBKeyboardHandler::_last_keys[6] = {0};

// ASCII conversion table for HID scan codes
static const char ascii_table[128][2] = {
    // [scan_code][0] = without shift, [1] = with shift
    {0, 0},           // 0x00
    {0, 0},           // 0x01
    {0, 0},           // 0x02
    {0, 0},           // 0x03
    {'a', 'A'},       // 0x04 - A
    {'b', 'B'},       // 0x05 - B
    {'c', 'C'},       // 0x06 - C
    {'d', 'D'},       // 0x07 - D
    {'e', 'E'},       // 0x08 - E
    {'f', 'F'},       // 0x09 - F
    {'g', 'G'},       // 0x0A - G
    {'h', 'H'},       // 0x0B - H
    {'i', 'I'},       // 0x0C - I
    {'j', 'J'},       // 0x0D - J
    {'k', 'K'},       // 0x0E - K
    {'l', 'L'},       // 0x0F - L
    {'m', 'M'},       // 0x10 - M
    {'n', 'N'},       // 0x11 - N
    {'o', 'O'},       // 0x12 - O
    {'p', 'P'},       // 0x13 - P
    {'q', 'Q'},       // 0x14 - Q
    {'r', 'R'},       // 0x15 - R
    {'s', 'S'},       // 0x16 - S
    {'t', 'T'},       // 0x17 - T
    {'u', 'U'},       // 0x18 - U
    {'v', 'V'},       // 0x19 - V
    {'w', 'W'},       // 0x1A - W
    {'x', 'X'},       // 0x1B - X
    {'y', 'Y'},       // 0x1C - Y
    {'z', 'Z'},       // 0x1D - Z
    {'1', '!'},       // 0x1E - 1
    {'2', '@'},       // 0x1F - 2
    {'3', '#'},       // 0x20 - 3
    {'4', '$'},       // 0x21 - 4
    {'5', '%'},       // 0x22 - 5
    {'6', '^'},       // 0x23 - 6
    {'7', '&'},       // 0x24 - 7
    {'8', '*'},       // 0x25 - 8
    {'9', '('},       // 0x26 - 9
    {'0', ')'},       // 0x27 - 0
    {'\n', '\n'},     // 0x28 - Enter
    {0x1B, 0x1B},     // 0x29 - Escape
    {0x08, 0x08},     // 0x2A - Backspace
    {'\t', '\t'},     // 0x2B - Tab
    {' ', ' '},       // 0x2C - Space
    {'-', '_'},       // 0x2D - Minus
    {'=', '+'},       // 0x2E - Equal
    {'[', '{'},       // 0x2F - [
    {']', '}'},       // 0x30 - ]
    {'\\', '|'},      // 0x31 - \
    {0, 0},           // 0x32
    {';', ':'},       // 0x33 - ;
    {'\'', '"'},      // 0x34 - '
    {'`', '~'},       // 0x35 - `
    {',', '<'},       // 0x36 - ,
    {'.', '>'},       // 0x37 - .
    {'/', '?'},       // 0x38 - /
    {0, 0},           // 0x39 - Caps Lock
};

// Scan code name table for debugging
static const char* scan_code_names[256] = {
    [0x04] = "A", [0x05] = "B", [0x06] = "C", [0x07] = "D", [0x08] = "E",
    [0x09] = "F", [0x0A] = "G", [0x0B] = "H", [0x0C] = "I", [0x0D] = "J",
    [0x0E] = "K", [0x0F] = "L", [0x10] = "M", [0x11] = "N", [0x12] = "O",
    [0x13] = "P", [0x14] = "Q", [0x15] = "R", [0x16] = "S", [0x17] = "T",
    [0x18] = "U", [0x19] = "V", [0x1A] = "W", [0x1B] = "X", [0x1C] = "Y",
    [0x1D] = "Z", [0x1E] = "1", [0x1F] = "2", [0x20] = "3", [0x21] = "4",
    [0x22] = "5", [0x23] = "6", [0x24] = "7", [0x25] = "8", [0x26] = "9",
    [0x27] = "0", [0x28] = "Enter", [0x29] = "Escape", [0x2A] = "Backspace",
    [0x2B] = "Tab", [0x2C] = "Space", [0x2D] = "Minus", [0x2E] = "Equal",
    [0x2F] = "[", [0x30] = "]", [0x31] = "\\", [0x33] = ";", [0x34] = "'",
    [0x35] = "`", [0x36] = ",", [0x37] = ".", [0x38] = "/", [0x39] = "CapsLock",
    [0x3A] = "F1", [0x3B] = "F2", [0x3C] = "F3", [0x3D] = "F4", [0x3E] = "F5",
    [0x3F] = "F6", [0x40] = "F7", [0x41] = "F8", [0x42] = "F9", [0x43] = "F10",
    [0x44] = "F11", [0x45] = "F12", [0x49] = "Insert", [0x4A] = "Home",
    [0x4B] = "PageUp", [0x4C] = "Delete", [0x4D] = "End", [0x4E] = "PageDown",
    [0x4F] = "Right", [0x50] = "Left", [0x51] = "Down", [0x52] = "Up",
};

// TinyUSB HID report callback
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if (len < 8) return;
    
    uint8_t modifier = report[0];
    uint8_t key_code = report[2];
    
    // Check for key press (scan code != 0)
    if (key_code != 0) {
        keyboard_event_t event;
        event.modifier = modifier;
        event.scan_code = key_code;
        event.is_pressed = true;
        event.ascii_char = USBKeyboardHandler::scanCodeToAscii(key_code, modifier, modifier & (HID_MOD_LSHIFT | HID_MOD_RSHIFT));
        
        if (USBKeyboardHandler::_callback) {
            USBKeyboardHandler::_callback(event);
        }
        
        Serial.printf("Key pressed: %s (0x%02X) - ASCII: %c\n", 
                      USBKeyboardHandler::scanCodeToName(key_code), key_code, 
                      event.ascii_char ? event.ascii_char : '?');
    }
}

// TinyUSB mount callback
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    Serial.println("HID device mounted!");
    USBKeyboardHandler::_keyboard_connected = true;
    tuh_hid_report_received_cb(dev_addr, instance, (const uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00", 8);
}

// TinyUSB unmount callback
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    Serial.println("HID device unmounted!");
    USBKeyboardHandler::_keyboard_connected = false;
}

bool USBKeyboardHandler::begin() {
    Serial.println("Initializing USB Keyboard Handler...");
    
    // Initialize TinyUSB
    tusb_init();
    
    Serial.println("USB Keyboard Handler initialized");
    return true;
}

void USBKeyboardHandler::end() {
    _keyboard_connected = false;
    _callback = nullptr;
}

void USBKeyboardHandler::onKeyboardEvent(KeyboardCallback callback) {
    _callback = callback;
}

void USBKeyboardHandler::update() {
    tuh_task();
}

bool USBKeyboardHandler::isKeyboardConnected() {
    return _keyboard_connected;
}

char USBKeyboardHandler::scanCodeToAscii(uint8_t scan_code, uint8_t modifier, bool shift_pressed) {
    if (scan_code >= 128) return 0;
    
    int index = shift_pressed ? 1 : 0;
    return ascii_table[scan_code][index];
}

const char* USBKeyboardHandler::scanCodeToName(uint8_t scan_code) {
    if (scan_code_names[scan_code]) {
        return scan_code_names[scan_code];
    }
    return "Unknown";
}
