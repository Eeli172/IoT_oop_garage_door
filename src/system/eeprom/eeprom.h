#ifndef IOT_OOP_GARAGE_DOOR_EEPROM_H
#define IOT_OOP_GARAGE_DOOR_EEPROM_H

#include <cstdint>
#include <optional>
#include <span>

class Eeprom {
public:
    Eeprom();

    /*
    Write 2 bytes: 1 byte original and inverted
    Return: true (succeed)  ||  false (failed)
    */
    bool write_byte(uint16_t memory_address, uint8_t data);

    /*
    Read 2 bytes: value & ~value @ memory_address and validate. 
    Return:  validated_value (succeed)  ||  std::nullopt (failure)
    */
    std::optional<uint8_t> read_byte(uint16_t memory_address) const;


    /*
    Write 8 bytes: original 4-byte float and its inverted value
    Return:  true (succeed)  ||  false (failed)
    */
    bool write_float(uint16_t memory_address, float data);

    /*
    Read 8 bytes: value & ~value @ memory_address. 
    Return:  validated_value (succeed)  ||  std::nullopt (failure)
    */
    std::optional<float> read_float(uint16_t memory_address) const;

    
private:
    /*
    Write --payload-- into eeprom.
    Return: true (succeed)  ||  false (failed)
    */
    bool save_data(std::span<const uint8_t> payload) const;

    /*
    Read data from --memory_address-- into --destination-- 
    Return: true (succeed)  ||  false (failed)
    */ 
    bool read_data(uint16_t memory_address, std::span<uint8_t> destination) const;

    /*
    Poll ACK until:
    answer from eeprom == eeprom is free from previous write
    --timeout_ms-- has passed
    Return:  true (ACK received)  ||  false (timeout)
    */
    bool poll_ack_with_timeout_ms(uint32_t timeout_ms) const;

    /*
    Initialize I2C hardware
    */
    void init_i2c_hardware();
};

#endif //IOT_OOP_GARAGE_DOOR_EEPROM_H