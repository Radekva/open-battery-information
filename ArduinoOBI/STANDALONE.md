# Arduino OBI - Standalone Mode

This guide explains how to use Arduino OBI in standalone mode, allowing you to communicate with batteries directly from the Arduino without requiring a PC running the Python GUI application.

## Overview

The BatteryOBI library provides a simple API for communicating with Makita LXT batteries directly from Arduino. This eliminates the need for a PC after the initial setup and programming.

## What's New

Previously, Arduino OBI required:
1. Arduino connected to PC via USB
2. Python GUI application running on PC
3. PC sending commands to Arduino

Now with standalone mode:
1. Arduino runs independently (can be powered by any USB power source)
2. Commands are executed directly on Arduino
3. Results displayed via Serial Monitor or processed by custom code

## Hardware Requirements

- Arduino Uno or Arduino Nano
- Circuit built according to the schematic in `docs/images/arduino-obi.png`
- OneWire interface on pin 6
- Enable control on pin 8
- USB cable for programming and serial communication

## Getting Started

### 1. Install the Library

The BatteryOBI library is already included in the ArduinoOBI project under `lib/BatteryOBI/`.

### 2. Choose an Example

Two example sketches are provided:

#### StandaloneMode.ino
Interactive menu-driven interface. Best for manual battery diagnostics and testing.
- Location: `examples/StandaloneMode/`
- Features: Manual command input, comprehensive testing options

#### AutomaticCheck.ino  
Automatic battery detection and status checking. Best for automated workflows.
- Location: `examples/AutomaticCheck/`
- Features: Automatic detection, error clearing prompts

### 3. Upload the Sketch

1. Open VS Code with PlatformIO
2. Open the ArduinoOBI project folder
3. Copy your chosen example to `src/main.cpp` (or create a new platformio.ini environment)
4. Build and upload to your Arduino

**Alternative:** If you want to keep the original USB bridge functionality, you can create a separate PlatformIO environment:

Add to `platformio.ini`:
```ini
[env:standalone]
platform = atmelavr
board = uno
framework = arduino
build_flags = -DSTANDALONE_MODE
```

Then copy the example to a new source file.

### 4. Use the Interface

Open Serial Monitor at 9600 baud and follow the on-screen instructions.

## Library API Reference

### Initialization

```cpp
#include <BatteryOBI.h>

// Create battery interface
BatteryOBI battery(ONEWIRE_PIN, ENABLE_PIN);

void setup() {
    battery.begin();  // Initialize the interface
}
```

### Reading Battery Information

```cpp
BatteryData data;

// Read battery model
char model[16];
if (battery.readModel(model)) {
    Serial.println(model);
}

// Read battery info (ROM ID, charge count, lock status, etc.)
if (battery.readBatteryInfo(&data)) {
    Serial.print("Charge count: ");
    Serial.println(data.chargeCount);
    Serial.print("Lock status: ");
    Serial.println(data.isLocked ? "LOCKED" : "UNLOCKED");
}

// Read battery voltages and temperatures
if (battery.readBatteryData(&data)) {
    Serial.print("Pack voltage: ");
    Serial.print(data.packVoltage / 1000.0);
    Serial.println(" V");
}

// Print all data in formatted output
battery.printBatteryData(&data);
```

### Battery Operations

```cpp
// Turn LEDs on (test mode)
if (battery.ledsOn()) {
    Serial.println("LEDs ON");
}

// Turn LEDs off
if (battery.ledsOff()) {
    Serial.println("LEDs OFF");
}

// Clear battery errors (unlock battery)
if (battery.clearErrors()) {
    Serial.println("Errors cleared");
}
```

### Error Handling

```cpp
if (!battery.readModel(model)) {
    Serial.print("Error: ");
    Serial.println(battery.getLastError());
}
```

## BatteryData Structure

```cpp
struct BatteryData {
    char model[16];              // Battery model string
    char romId[24];              // ROM ID string
    uint16_t chargeCount;        // Number of charge cycles
    bool isLocked;               // Lock status
    uint8_t statusCode;          // Error/status code
    uint16_t packVoltage;        // Pack voltage in mV
    uint16_t cell1Voltage;       // Cell 1 voltage in mV
    uint16_t cell2Voltage;       // Cell 2 voltage in mV
    uint16_t cell3Voltage;       // Cell 3 voltage in mV
    uint16_t cell4Voltage;       // Cell 4 voltage in mV
    uint16_t cell5Voltage;       // Cell 5 voltage in mV
    int16_t tempSensor1;         // Temperature in 0.01°C
    int16_t tempSensor2;         // Temperature in 0.01°C
    uint8_t manufacturingYear;   // Manufacturing year (2-digit)
    uint8_t manufacturingMonth;  // Manufacturing month
    uint8_t manufacturingDay;    // Manufacturing day
    uint8_t capacity;            // Capacity in 0.1 Ah
    uint8_t batteryType;         // Battery type code
};
```

## Practical Use Cases

### 1. Battery Diagnostics Station
Set up a dedicated station with Arduino powered by a wall adapter. Users can connect batteries and view diagnostics on Serial Monitor or connected display.

### 2. Automated Battery Testing
Use AutomaticCheck example to automatically test batteries in a production or repair environment.

### 3. Battery Unlock Service
Provide a simple service to unlock batteries with temporary faults without needing a PC.

### 4. Custom Applications
Build custom battery management systems using the BatteryOBI library as a foundation.

## Maintaining USB Bridge Mode

The original USB bridge functionality (for use with the Python GUI) remains available in `src/main.cpp`. You can switch between modes by:

1. Uploading different sketches
2. Using compile-time flags to select mode
3. Creating separate PlatformIO environments

## Troubleshooting

### Battery Not Detected
- Check all connections according to the schematic
- Verify battery contacts are clean
- Ensure battery is charged enough to respond

### Commands Fail
- Check ENABLE_PIN timing (400ms delay is required)
- Verify OneWire signal integrity
- Some battery models may require different command sequences

### Compilation Errors
- Ensure OneWire library is available in `lib/OneWire/`
- Verify BatteryOBI library is in `lib/BatteryOBI/`
- Check that you're using Arduino framework in PlatformIO

## Contributing

If you develop improvements to the standalone mode or support additional battery models, please contribute back to the project!

## Support

For questions or issues:
- Email: openbatteryinformation@gmail.com
- GitHub Issues: https://github.com/mnh-jansson/open-battery-information/issues

## License

This code is part of the Open Battery Information project and follows the same license terms as the main project.
