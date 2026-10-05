#ifndef IOT_OOP_GARAGE_DOOR_EEPROM_H
#define IOT_OOP_GARAGE_DOOR_EEPROM_H

#include <cstdint>
#include <optional>
#include <span>

class Eeprom {
public:
    Eeprom();

    // write and read one byte
    bool write_byte(uint16_t memory_address, uint8_t data);
    std::optional<uint8_t> read_byte(uint16_t memory_address) const;

    // write and read four bytes
    bool write_float(uint16_t memory_address, float data);
    std::optional<float> read_float(uint16_t memory_address) const;

private:
    bool save_data(std::span<const uint8_t> payload) const;
    bool read_data(uint16_t memory_address, std::span<uint8_t> destination) const;
    void init_i2c_hardware();
    bool poll_ack_with_timeout_ms(uint32_t timeout_ms) const;
};

#endif //IOT_OOP_GARAGE_DOOR_EEPROM_H