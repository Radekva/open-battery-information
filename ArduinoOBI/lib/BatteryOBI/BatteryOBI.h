/**
 * @file BatteryOBI.h
 * @brief Library for direct battery communication without PC
 * 
 * This library allows Arduino to communicate with batteries directly,
 * eliminating the need for a PC running the Python GUI application.
 */

#ifndef BATTERY_OBI_H
#define BATTERY_OBI_H

#include <Arduino.h>
#include "OneWire2.h"

// Makita LXT Command Definitions
namespace MakitaLXT {
    // ROM and Message Commands
    const uint8_t READ_MSG_CMD[] = {0xAA, 0x00};
    const uint8_t READ_MSG_LEN = 2;
    const uint8_t READ_MSG_RESPONSE_LEN = 40;
    
    // Model Commands
    const uint8_t MODEL_CMD[] = {0xDC, 0x0C};
    const uint8_t MODEL_CMD_LEN = 2;
    const uint8_t MODEL_RESPONSE_LEN = 16;
    
    // Data Reading Commands
    const uint8_t READ_DATA_CMD[] = {0xD7, 0x00, 0x00, 0xFF};
    const uint8_t READ_DATA_LEN = 4;
    const uint8_t READ_DATA_RESPONSE_LEN = 29;
    
    // Test Mode and LED Commands
    const uint8_t TESTMODE_CMD[] = {0xD9, 0x96, 0xA5};
    const uint8_t TESTMODE_LEN = 3;
    const uint8_t LEDS_ON_CMD[] = {0xDA, 0x31};
    const uint8_t LEDS_ON_LEN = 2;
    const uint8_t LEDS_OFF_CMD[] = {0xDA, 0x34};
    const uint8_t LEDS_OFF_LEN = 2;
    
    // Reset Commands
    const uint8_t RESET_ERROR_CMD[] = {0xDA, 0x04};
    const uint8_t RESET_ERROR_LEN = 2;
    
    // F0513 Specific Commands
    const uint8_t F0513_VCELL_1_CMD = 0x31;
    const uint8_t F0513_VCELL_2_CMD = 0x32;
    const uint8_t F0513_VCELL_3_CMD = 0x33;
    const uint8_t F0513_VCELL_4_CMD = 0x34;
    const uint8_t F0513_VCELL_5_CMD = 0x35;
    const uint8_t F0513_TEMP_CMD = 0x52;
    const uint8_t F0513_MODEL_CMD = 0x31;
    const uint8_t F0513_VERSION_CMD = 0x32;
    const uint8_t F0513_TESTMODE_CMD = 0x99;
}

/**
 * @brief Battery data structure
 */
struct BatteryData {
    char model[16];
    char romId[24];
    uint16_t chargeCount;
    bool isLocked;
    uint8_t statusCode;
    uint16_t packVoltage;      // in millivolts
    uint16_t cell1Voltage;     // in millivolts
    uint16_t cell2Voltage;     // in millivolts
    uint16_t cell3Voltage;     // in millivolts
    uint16_t cell4Voltage;     // in millivolts
    uint16_t cell5Voltage;     // in millivolts
    int16_t tempSensor1;       // in 0.01°C
    int16_t tempSensor2;       // in 0.01°C
    uint8_t manufacturingYear;
    uint8_t manufacturingMonth;
    uint8_t manufacturingDay;
    uint8_t capacity;          // in 0.1 Ah
    uint8_t batteryType;
};

/**
 * @brief Main class for battery communication
 */
class BatteryOBI {
public:
    /**
     * @brief Constructor
     * @param oneWirePin Pin number for OneWire communication
     * @param enablePin Pin number for enable/RTS control
     */
    BatteryOBI(uint8_t oneWirePin, uint8_t enablePin);
    
    /**
     * @brief Initialize the battery interface
     */
    void begin();
    
    /**
     * @brief Enable communication (set RTS high)
     */
    void enable();
    
    /**
     * @brief Disable communication (set RTS low)
     */
    void disable();
    
    /**
     * @brief Read battery model
     * @param model Buffer to store model string (min 16 bytes)
     * @return true if successful
     */
    bool readModel(char* model);
    
    /**
     * @brief Read battery message and ROM ID
     * @param data Pointer to BatteryData structure
     * @return true if successful
     */
    bool readBatteryInfo(BatteryData* data);
    
    /**
     * @brief Read battery voltages and temperatures
     * @param data Pointer to BatteryData structure
     * @return true if successful
     */
    bool readBatteryData(BatteryData* data);
    
    /**
     * @brief Turn battery LEDs on (test mode)
     * @return true if successful
     */
    bool ledsOn();
    
    /**
     * @brief Turn battery LEDs off
     * @return true if successful
     */
    bool ledsOff();
    
    /**
     * @brief Clear battery errors
     * @return true if successful
     */
    bool clearErrors();
    
    /**
     * @brief Print battery data to Serial
     * @param data Pointer to BatteryData structure
     */
    void printBatteryData(const BatteryData* data);
    
    /**
     * @brief Get last error message
     * @return Error message string
     */
    const char* getLastError();

private:
    OneWire _oneWire;
    uint8_t _enablePin;
    char _lastError[64];
    
    /**
     * @brief Send command with 0x33 prefix
     */
    bool cmdAndRead33(const uint8_t* cmd, uint8_t cmdLen, uint8_t* rsp, uint8_t rspLen);
    
    /**
     * @brief Send command with 0xCC prefix
     */
    bool cmdAndReadCC(const uint8_t* cmd, uint8_t cmdLen, uint8_t* rsp, uint8_t rspLen);
    
    /**
     * @brief Send command with 0xCC and 0x99 prefix (for some models)
     */
    bool cmdWithCCandRead(const uint8_t* cmd, uint8_t cmdLen, uint8_t* rsp, uint8_t rspLen);
    
    /**
     * @brief Swap nibbles in a byte
     */
    uint8_t nibbleSwap(uint8_t byte);
    
    /**
     * @brief Set last error message
     */
    void setError(const char* msg);
};

#endif // BATTERY_OBI_H
