#ifndef IOT_OOP_GARAGE_DOOR_EEPROM_H
#define IOT_OOP_GARAGE_DOOR_EEPROM_H

/* STL */
#include <cstdint>
#include <optional>
#include <span>

class Eeprom {
public:

    Eeprom();

    // ========================================================================
    // EEPROM OPERATIONS
    // ========================================================================

    /**
     * @brief Writes an 8-bit (byte) payload to a 16-bit (2-byte) address.
     * @param memory_address The memory address of where the write starts from, written in bursts.
     * @param data Payload for the data.
     * @return true if write was successful, false otherwise.
     */
    bool write_byte(uint16_t memory_address, uint8_t data);

    /**
     * @brief Reads an 8-bit (byte) value off of the specified memory address.
     * @param memory_address Determines the address from which to read off of.
     * @return Total bytes read if read was successful, std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<uint8_t> read_byte(uint16_t memory_address) const;

    /**
     * @brief Writes a floating point number (64-bit, 8-byte) to the EEPROM chip.
     * @param memory_address Determines the address to write to.
     * @param data Payload for the data.
     * @return true if write was successful, false otherwise.
     */
    bool write_float(uint16_t memory_address, float data);

    /**
     * @brief Reads a floating point number from the EEPROM chip.
     * @param memory_address Determines the address from which to read off of.
     * @return Total bytes read if read was successful, std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<float> read_float(uint16_t memory_address) const;

    
private:

    /**
     * @brief Takes the entire data payload (original data + inverted version)
     * and writes it using the class' method 'write_byte'
     * @param payload To store the packaged data.
     * @return true if write was successful, false otherwise.
     */
    bool save_data(std::span<const uint8_t> payload) const;

    /**
     * @brief Reads the data in a burst starting from the specified memory address
     * until the destination (total steps / length of the operation) has been reached.
     * @param memory_address Determines the address from which the operation starts from.
     * @param destination Total step length (e.g, 4 indicates that a value is read from a total of 4 addresses)
     * @return true if read was successful, false otherwise.
     */
    bool read_data(uint16_t memory_address, std::span<uint8_t> destination) const;

    /**
     * @brief Polls the EEPROM chip for indication of being ready from an operation.
     * @param timeout_ms Total time to wait for an answer from the chip.
     * @return true if ACK was received, false otherwise (timeout)
     */
    bool poll_ack_with_timeout_ms(uint32_t timeout_ms) const;

    /**
     * @brief Initializes I2C bus architecture for read/write transfers to/from the EEPROM chip.
     */
    void init_i2c_hardware();
};

#endif //IOT_OOP_GARAGE_DOOR_EEPROM_H