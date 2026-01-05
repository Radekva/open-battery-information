/**
 * @file BatteryOBI.cpp
 * @brief Implementation of BatteryOBI library
 */

#include "BatteryOBI.h"

BatteryOBI::BatteryOBI(uint8_t oneWirePin, uint8_t enablePin)
    : _oneWire(oneWirePin), _enablePin(enablePin) {
    _lastError[0] = '\0';
}

void BatteryOBI::begin() {
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_enablePin, LOW);
}

void BatteryOBI::enable() {
    digitalWrite(_enablePin, HIGH);
    delay(400);
}

void BatteryOBI::disable() {
    digitalWrite(_enablePin, LOW);
}

bool BatteryOBI::cmdAndRead33(const uint8_t* cmd, uint8_t cmdLen, uint8_t* rsp, uint8_t rspLen) {
    _oneWire.reset();
    delayMicroseconds(400);
    _oneWire.write(0x33, 0);
    
    // Read ROM ID (8 bytes)
    for (int i = 0; i < 8; i++) {
        delayMicroseconds(90);
        rsp[i] = _oneWire.read();
    }
    
    // Send command
    for (int i = 0; i < cmdLen; i++) {
        delayMicroseconds(90);
        _oneWire.write(cmd[i], 0);
    }
    
    // Read response
    for (int i = 8; i < rspLen + 8; i++) {
        delayMicroseconds(90);
        rsp[i] = _oneWire.read();
    }
    
    return true;
}

bool BatteryOBI::cmdAndReadCC(const uint8_t* cmd, uint8_t cmdLen, uint8_t* rsp, uint8_t rspLen) {
    _oneWire.reset();
    delayMicroseconds(400);
    _oneWire.write(0xCC, 0);
    
    // Send command
    for (int i = 0; i < cmdLen; i++) {
        delayMicroseconds(90);
        _oneWire.write(cmd[i], 0);
    }
    
    // Read response
    for (int i = 0; i < rspLen; i++) {
        delayMicroseconds(90);
        rsp[i] = _oneWire.read();
    }
    
    return true;
}

bool BatteryOBI::cmdWithCCandRead(const uint8_t* cmd, uint8_t cmdLen, uint8_t* rsp, uint8_t rspLen) {
    _oneWire.reset();
    delayMicroseconds(400);
    _oneWire.write(0xCC, 0);
    delayMicroseconds(90);
    _oneWire.write(0x99, 0);
    delay(400);
    
    _oneWire.reset();
    delayMicroseconds(400);
    
    // Send command
    for (int i = 0; i < cmdLen; i++) {
        delayMicroseconds(90);
        _oneWire.write(cmd[i], 0);
    }
    
    // Read response
    for (int i = 0; i < rspLen; i++) {
        delayMicroseconds(90);
        rsp[i] = _oneWire.read();
    }
    
    return true;
}

uint8_t BatteryOBI::nibbleSwap(uint8_t byte) {
    uint8_t upperNibble = (byte & 0xF0) >> 4;
    uint8_t lowerNibble = (byte & 0x0F) << 4;
    return upperNibble | lowerNibble;
}

void BatteryOBI::setError(const char* msg) {
    strncpy(_lastError, msg, sizeof(_lastError) - 1);
    _lastError[sizeof(_lastError) - 1] = '\0';
}

const char* BatteryOBI::getLastError() {
    return _lastError;
}

bool BatteryOBI::readModel(char* model) {
    uint8_t response[32];
    
    enable();
    
    if (!cmdAndReadCC(MakitaLXT::MODEL_CMD, MakitaLXT::MODEL_CMD_LEN, 
                      response, MakitaLXT::MODEL_RESPONSE_LEN)) {
        disable();
        setError("Failed to read model");
        return false;
    }
    
    disable();
    
    // Copy model string (7 bytes)
    for (int i = 0; i < 7 && i < 15; i++) {
        model[i] = (char)response[i];
    }
    model[7] = '\0';
    
    return true;
}

bool BatteryOBI::readBatteryInfo(BatteryData* data) {
    uint8_t response[48];
    
    enable();
    
    if (!cmdAndRead33(MakitaLXT::READ_MSG_CMD, MakitaLXT::READ_MSG_LEN, 
                      response, MakitaLXT::READ_MSG_RESPONSE_LEN)) {
        disable();
        setError("Failed to read battery info");
        return false;
    }
    
    disable();
    
    // Parse ROM ID (first 8 bytes)
    snprintf(data->romId, sizeof(data->romId), 
             "%02X %02X %02X %02X %02X %02X %02X %02X",
             response[0], response[1], response[2], response[3],
             response[4], response[5], response[6], response[7]);
    
    // Parse manufacturing date (bytes 8-10: year, month, day)
    data->manufacturingYear = response[10];
    data->manufacturingMonth = response[11];
    data->manufacturingDay = response[12];
    
    // Parse charge count (bytes 37-38, nibble swapped)
    uint8_t swapped1 = nibbleSwap(response[45]);
    uint8_t swapped2 = nibbleSwap(response[44]);
    uint16_t chargeCount = ((uint16_t)swapped1 << 8) | swapped2;
    data->chargeCount = chargeCount & 0x0FFF;
    
    // Parse lock status (byte 30, lower nibble)
    uint8_t lockNibble = response[38] & 0x0F;
    data->isLocked = (lockNibble > 0);
    
    // Parse status code (byte 29)
    data->statusCode = response[37];
    
    // Parse capacity (byte 26, nibble swapped, in 0.1 Ah)
    data->capacity = nibbleSwap(response[34]);
    
    // Parse battery type (byte 21, nibble swapped)
    data->batteryType = nibbleSwap(response[29]);
    
    return true;
}

bool BatteryOBI::readBatteryData(BatteryData* data) {
    uint8_t response[32];
    
    enable();
    
    if (!cmdAndReadCC(MakitaLXT::READ_DATA_CMD, MakitaLXT::READ_DATA_LEN, 
                      response, MakitaLXT::READ_DATA_RESPONSE_LEN)) {
        disable();
        setError("Failed to read battery data");
        return false;
    }
    
    disable();
    
    // Parse voltages (little endian, in millivolts)
    data->packVoltage = ((uint16_t)response[1] << 8) | response[0];
    data->cell1Voltage = ((uint16_t)response[3] << 8) | response[2];
    data->cell2Voltage = ((uint16_t)response[5] << 8) | response[4];
    data->cell3Voltage = ((uint16_t)response[7] << 8) | response[6];
    data->cell4Voltage = ((uint16_t)response[9] << 8) | response[8];
    data->cell5Voltage = ((uint16_t)response[11] << 8) | response[10];
    
    // Parse temperatures (little endian, in 0.01°C)
    data->tempSensor1 = ((int16_t)response[15] << 8) | response[14];
    data->tempSensor2 = ((int16_t)response[17] << 8) | response[16];
    
    return true;
}

bool BatteryOBI::ledsOn() {
    uint8_t response[16];
    
    enable();
    
    // Enter test mode
    if (!cmdAndRead33(MakitaLXT::TESTMODE_CMD, MakitaLXT::TESTMODE_LEN, 
                      response, 9)) {
        disable();
        setError("Failed to enter test mode");
        return false;
    }
    
    // Turn LEDs on
    if (!cmdAndRead33(MakitaLXT::LEDS_ON_CMD, MakitaLXT::LEDS_ON_LEN, 
                      response, 9)) {
        disable();
        setError("Failed to turn LEDs on");
        return false;
    }
    
    disable();
    return true;
}

bool BatteryOBI::ledsOff() {
    uint8_t response[16];
    
    enable();
    
    // Enter test mode
    if (!cmdAndRead33(MakitaLXT::TESTMODE_CMD, MakitaLXT::TESTMODE_LEN, 
                      response, 9)) {
        disable();
        setError("Failed to enter test mode");
        return false;
    }
    
    // Turn LEDs off
    if (!cmdAndRead33(MakitaLXT::LEDS_OFF_CMD, MakitaLXT::LEDS_OFF_LEN, 
                      response, 9)) {
        disable();
        setError("Failed to turn LEDs off");
        return false;
    }
    
    disable();
    return true;
}

bool BatteryOBI::clearErrors() {
    uint8_t response[16];
    
    enable();
    
    // Enter test mode
    if (!cmdAndRead33(MakitaLXT::TESTMODE_CMD, MakitaLXT::TESTMODE_LEN, 
                      response, 9)) {
        disable();
        setError("Failed to enter test mode");
        return false;
    }
    
    // Clear errors
    if (!cmdAndRead33(MakitaLXT::RESET_ERROR_CMD, MakitaLXT::RESET_ERROR_LEN, 
                      response, 9)) {
        disable();
        setError("Failed to clear errors");
        return false;
    }
    
    disable();
    return true;
}

void BatteryOBI::printBatteryData(const BatteryData* data) {
    Serial.println(F("=== Battery Information ==="));
    
    if (data->model[0] != '\0') {
        Serial.print(F("Model: "));
        Serial.println(data->model);
    }
    
    if (data->romId[0] != '\0') {
        Serial.print(F("ROM ID: "));
        Serial.println(data->romId);
    }
    
    if (data->chargeCount > 0) {
        Serial.print(F("Charge Count: "));
        Serial.println(data->chargeCount);
    }
    
    Serial.print(F("Lock Status: "));
    Serial.println(data->isLocked ? F("LOCKED") : F("UNLOCKED"));
    
    if (data->statusCode > 0) {
        Serial.print(F("Status Code: 0x"));
        Serial.println(data->statusCode, HEX);
    }
    
    if (data->manufacturingYear > 0) {
        Serial.print(F("Manufacturing Date: "));
        Serial.print(data->manufacturingDay);
        Serial.print(F("/"));
        Serial.print(data->manufacturingMonth);
        Serial.print(F("/20"));
        Serial.println(data->manufacturingYear);
    }
    
    if (data->capacity > 0) {
        Serial.print(F("Capacity: "));
        Serial.print(data->capacity / 10.0, 1);
        Serial.println(F(" Ah"));
    }
    
    if (data->batteryType > 0) {
        Serial.print(F("Battery Type: "));
        Serial.println(data->batteryType);
    }
    
    if (data->packVoltage > 0) {
        Serial.println();
        Serial.println(F("=== Voltage Data ==="));
        Serial.print(F("Pack Voltage: "));
        Serial.print(data->packVoltage / 1000.0, 3);
        Serial.println(F(" V"));
        
        Serial.print(F("Cell 1: "));
        Serial.print(data->cell1Voltage / 1000.0, 3);
        Serial.println(F(" V"));
        
        Serial.print(F("Cell 2: "));
        Serial.print(data->cell2Voltage / 1000.0, 3);
        Serial.println(F(" V"));
        
        Serial.print(F("Cell 3: "));
        Serial.print(data->cell3Voltage / 1000.0, 3);
        Serial.println(F(" V"));
        
        Serial.print(F("Cell 4: "));
        Serial.print(data->cell4Voltage / 1000.0, 3);
        Serial.println(F(" V"));
        
        Serial.print(F("Cell 5: "));
        Serial.print(data->cell5Voltage / 1000.0, 3);
        Serial.println(F(" V"));
        
        // Calculate cell voltage difference
        uint16_t minVoltage = data->cell1Voltage;
        uint16_t maxVoltage = data->cell1Voltage;
        
        if (data->cell2Voltage < minVoltage) minVoltage = data->cell2Voltage;
        if (data->cell3Voltage < minVoltage) minVoltage = data->cell3Voltage;
        if (data->cell4Voltage < minVoltage) minVoltage = data->cell4Voltage;
        if (data->cell5Voltage < minVoltage) minVoltage = data->cell5Voltage;
        
        if (data->cell2Voltage > maxVoltage) maxVoltage = data->cell2Voltage;
        if (data->cell3Voltage > maxVoltage) maxVoltage = data->cell3Voltage;
        if (data->cell4Voltage > maxVoltage) maxVoltage = data->cell4Voltage;
        if (data->cell5Voltage > maxVoltage) maxVoltage = data->cell5Voltage;
        
        Serial.print(F("Cell Voltage Difference: "));
        Serial.print((maxVoltage - minVoltage) / 1000.0, 3);
        Serial.println(F(" V"));
    }
    
    if (data->tempSensor1 != 0) {
        Serial.println();
        Serial.println(F("=== Temperature Data ==="));
        Serial.print(F("Temperature Sensor 1: "));
        Serial.print(data->tempSensor1 / 100.0, 2);
        Serial.println(F(" °C"));
        
        if (data->tempSensor2 != 0) {
            Serial.print(F("Temperature Sensor 2: "));
            Serial.print(data->tempSensor2 / 100.0, 2);
            Serial.println(F(" °C"));
        }
    }
    
    Serial.println(F("==========================="));
}
