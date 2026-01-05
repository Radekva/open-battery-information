/**
 * @file AutomaticCheck.ino
 * @brief Automatic battery check example
 * 
 * This example automatically checks battery status when connected
 * and can optionally clear errors if detected.
 * 
 * Hardware Setup:
 * - OneWire pin: 6
 * - Enable pin: 8
 * - Build the circuit according to the schematic in docs/images/arduino-obi.png
 * 
 * Usage:
 * 1. Upload this sketch to your Arduino
 * 2. Open Serial Monitor at 9600 baud
 * 3. Connect a battery to the interface
 * 4. The Arduino will automatically read and display battery information
 * 5. If the battery is locked, it will ask if you want to clear errors
 */

#include <BatteryOBI.h>

// Pin definitions
#define ONEWIRE_PIN 6
#define ENABLE_PIN 8

// Check interval (milliseconds)
#define CHECK_INTERVAL 5000

// Create battery interface
BatteryOBI battery(ONEWIRE_PIN, ENABLE_PIN);
BatteryData batteryData;
unsigned long lastCheck = 0;
bool batteryPresent = false;

void setup() {
    Serial.begin(9600);
    
    while (!Serial) {
        ;
    }
    
    Serial.println(F("\n================================="));
    Serial.println(F("Arduino OBI - Automatic Check"));
    Serial.println(F("================================="));
    Serial.println();
    
    battery.begin();
    Serial.println(F("Ready! Connect a battery..."));
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check battery every CHECK_INTERVAL milliseconds
    if (currentTime - lastCheck >= CHECK_INTERVAL) {
        lastCheck = currentTime;
        checkBattery();
    }
    
    // Check for user input
    if (Serial.available() > 0) {
        handleUserInput();
    }
}

void checkBattery() {
    // Try to read battery model
    if (battery.readModel(batteryData.model)) {
        if (!batteryPresent) {
            batteryPresent = true;
            Serial.println(F("\n*** Battery detected! ***\n"));
            readFullBatteryData();
        }
    } else {
        if (batteryPresent) {
            batteryPresent = false;
            Serial.println(F("\n*** Battery disconnected ***\n"));
        }
    }
}

void readFullBatteryData() {
    Serial.println(F("Reading battery data..."));
    Serial.println();
    
    // Read battery info
    if (battery.readBatteryInfo(&batteryData)) {
        // Read battery voltages and temperatures
        battery.readBatteryData(&batteryData);
        
        // Print all data
        battery.printBatteryData(&batteryData);
        
        // Check if battery is locked
        if (batteryData.isLocked) {
            Serial.println();
            Serial.println(F("⚠ WARNING: Battery is LOCKED!"));
            Serial.print(F("Status code: 0x"));
            Serial.println(batteryData.statusCode, HEX);
            Serial.println();
            Serial.println(F("Would you like to try clearing the error?"));
            Serial.println(F("Send 'y' to clear errors, any other key to skip"));
        } else {
            Serial.println();
            Serial.println(F("✓ Battery is UNLOCKED and ready to use"));
        }
    } else {
        Serial.print(F("Failed to read battery data: "));
        Serial.println(battery.getLastError());
    }
    
    Serial.println();
}

void handleUserInput() {
    char cmd = Serial.read();
    
    // Clear any extra characters
    while (Serial.available() > 0) {
        Serial.read();
    }
    
    if (cmd == 'y' || cmd == 'Y') {
        clearBatteryErrors();
    } else if (cmd == 'r' || cmd == 'R') {
        Serial.println(F("\nRe-reading battery data..."));
        readFullBatteryData();
    }
}

void clearBatteryErrors() {
    Serial.println();
    Serial.println(F("Attempting to clear battery errors..."));
    
    if (battery.clearErrors()) {
        Serial.println(F("✓ Clear command sent successfully"));
        Serial.println();
        
        // Wait a moment for the battery to reset
        delay(1000);
        
        // Re-read battery info to check status
        Serial.println(F("Checking battery status..."));
        if (battery.readBatteryInfo(&batteryData)) {
            if (batteryData.isLocked) {
                Serial.println(F("⚠ Battery is still locked"));
                Serial.println(F("The error may be permanent or require physical repair"));
            } else {
                Serial.println(F("✓ SUCCESS! Battery is now unlocked!"));
            }
            
            battery.printBatteryData(&batteryData);
        }
    } else {
        Serial.print(F("✗ Failed to clear errors: "));
        Serial.println(battery.getLastError());
    }
    
    Serial.println();
    Serial.println(F("Send 'r' to re-read battery data"));
}
