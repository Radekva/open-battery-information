/**
 * @file StandaloneMode.ino
 * @brief Example of using Arduino OBI in standalone mode
 * 
 * This example demonstrates how to use the BatteryOBI library
 * to communicate with Makita LXT batteries directly from Arduino
 * without requiring a PC.
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
 * 4. Send commands via Serial Monitor:
 *    - '1' or 'i' : Read battery information (model, ROM ID, charge count, etc.)
 *    - '2' or 'd' : Read battery data (voltages and temperatures)
 *    - '3' or 'a' : Read all data (info + voltages)
 *    - '4' or 'l' : LED test ON
 *    - '5' or 'o' : LED test OFF
 *    - '6' or 'c' : Clear battery errors
 *    - 'h' or '?' : Show help menu
 */

#include <BatteryOBI.h>

// Pin definitions (match ArduinoOBI hardware)
#define ONEWIRE_PIN 6
#define ENABLE_PIN 8

// Create battery interface
BatteryOBI battery(ONEWIRE_PIN, ENABLE_PIN);
BatteryData batteryData;

void setup() {
    Serial.begin(9600);
    
    // Wait for serial port to connect
    while (!Serial) {
        ; 
    }
    
    Serial.println(F("\n================================="));
    Serial.println(F("Arduino OBI - Standalone Mode"));
    Serial.println(F("================================="));
    Serial.println(F("Version: 1.0.0"));
    Serial.println();
    
    // Initialize battery interface
    battery.begin();
    Serial.println(F("Battery interface initialized"));
    
    // Show help menu
    showHelp();
}

void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        // Clear any extra characters
        while (Serial.available() > 0) {
            Serial.read();
        }
        
        Serial.println();
        
        switch (cmd) {
            case '1':
            case 'i':
            case 'I':
                readBatteryInfo();
                break;
                
            case '2':
            case 'd':
            case 'D':
                readBatteryData();
                break;
                
            case '3':
            case 'a':
            case 'A':
                readAllData();
                break;
                
            case '4':
            case 'l':
            case 'L':
                testLedsOn();
                break;
                
            case '5':
            case 'o':
            case 'O':
                testLedsOff();
                break;
                
            case '6':
            case 'c':
            case 'C':
                clearErrors();
                break;
                
            case 'h':
            case 'H':
            case '?':
                showHelp();
                break;
                
            case '\n':
            case '\r':
                // Ignore newlines
                break;
                
            default:
                Serial.print(F("Unknown command: "));
                Serial.println(cmd);
                Serial.println(F("Type 'h' for help"));
                break;
        }
        
        Serial.println();
        Serial.println(F("Ready for next command..."));
    }
}

void showHelp() {
    Serial.println(F("\n=== Available Commands ==="));
    Serial.println(F("1 or i - Read battery information"));
    Serial.println(F("2 or d - Read battery data (voltages & temps)"));
    Serial.println(F("3 or a - Read all data"));
    Serial.println(F("4 or l - LED test ON"));
    Serial.println(F("5 or o - LED test OFF"));
    Serial.println(F("6 or c - Clear battery errors"));
    Serial.println(F("h or ? - Show this help"));
    Serial.println(F("==========================\n"));
}

void readBatteryInfo() {
    Serial.println(F("Reading battery information..."));
    
    // Read model
    if (battery.readModel(batteryData.model)) {
        Serial.print(F("✓ Model read: "));
        Serial.println(batteryData.model);
    } else {
        Serial.print(F("✗ Failed to read model: "));
        Serial.println(battery.getLastError());
    }
    
    // Read battery info (ROM ID, charge count, etc.)
    if (battery.readBatteryInfo(&batteryData)) {
        Serial.println(F("✓ Battery info read successfully"));
        battery.printBatteryData(&batteryData);
    } else {
        Serial.print(F("✗ Failed to read battery info: "));
        Serial.println(battery.getLastError());
    }
}

void readBatteryData() {
    Serial.println(F("Reading battery data..."));
    
    if (battery.readBatteryData(&batteryData)) {
        Serial.println(F("✓ Battery data read successfully"));
        battery.printBatteryData(&batteryData);
    } else {
        Serial.print(F("✗ Failed to read battery data: "));
        Serial.println(battery.getLastError());
    }
}

void readAllData() {
    Serial.println(F("Reading all battery data..."));
    
    // Read model
    if (!battery.readModel(batteryData.model)) {
        Serial.print(F("✗ Failed to read model: "));
        Serial.println(battery.getLastError());
        return;
    }
    
    // Read battery info
    if (!battery.readBatteryInfo(&batteryData)) {
        Serial.print(F("✗ Failed to read battery info: "));
        Serial.println(battery.getLastError());
        return;
    }
    
    // Read battery data
    if (!battery.readBatteryData(&batteryData)) {
        Serial.print(F("✗ Failed to read battery data: "));
        Serial.println(battery.getLastError());
        return;
    }
    
    Serial.println(F("✓ All data read successfully"));
    battery.printBatteryData(&batteryData);
}

void testLedsOn() {
    Serial.println(F("Turning LEDs ON..."));
    
    if (battery.ledsOn()) {
        Serial.println(F("✓ LEDs turned ON"));
        Serial.println(F("Check if battery LEDs are lit"));
    } else {
        Serial.print(F("✗ Failed to turn LEDs on: "));
        Serial.println(battery.getLastError());
    }
}

void testLedsOff() {
    Serial.println(F("Turning LEDs OFF..."));
    
    if (battery.ledsOff()) {
        Serial.println(F("✓ LEDs turned OFF"));
    } else {
        Serial.print(F("✗ Failed to turn LEDs off: "));
        Serial.println(battery.getLastError());
    }
}

void clearErrors() {
    Serial.println(F("Clearing battery errors..."));
    
    if (battery.clearErrors()) {
        Serial.println(F("✓ Errors cleared successfully"));
        Serial.println(F("The battery should now be unlocked if the error was temporary"));
    } else {
        Serial.print(F("✗ Failed to clear errors: "));
        Serial.println(battery.getLastError());
    }
}
