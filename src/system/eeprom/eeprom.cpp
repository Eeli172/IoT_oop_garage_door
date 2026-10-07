#include "eeprom.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "../../macros/macros.h"
#include <array>
#include <cstring>
#include <iostream>



/* ############## P U B L I C ############## */

Eeprom::Eeprom() {
    init_i2c_hardware();
}

/*
Write 2 bytes: 1 byte original and inverted
Return:  true (succeed)  ||  false (failed)
*/
bool Eeprom::write_byte(uint16_t memory_address, uint8_t data) {
    std::array<uint8_t, EEPROM_PAYLOAD_SIZE_BYTE> payload = {
        static_cast<uint8_t>((memory_address >> BYTE_BITS) & 0xFF),
        static_cast<uint8_t>(memory_address & 0xFF),
        data,
        static_cast<uint8_t>(~data)
    };

    if (!save_data(payload)) {
        std::cerr << "[ERROR-EEPROM] --> write_byte: FAILED" << std::endl;
        return false;
    }

    return true;
}

/*
Read 2 bytes: value & ~value @ memory_address and validate. 
Return:  validated_value (succeed)  ||  std::nullopt (failure)
*/
std::optional<uint8_t> Eeprom::read_byte(uint16_t memory_address) const {
    std::array<uint8_t, EEPROM_BUFFER_SIZE_BYTE> buffer{};
    
    if (!read_data(memory_address, buffer)) {
        std::cerr << "[ERROR-EEPROM] --> read_byte: FAILED" << std::endl;
        return std::nullopt;
    }

    if (static_cast<uint8_t>(~buffer[0]) != buffer[1]) {
        std::cerr << "[ERROR-EEPROM] --> read_byte: INVALID DATA" << std::endl;
        return std::nullopt;
    }

    return buffer[0];
}



/*
Write 8 bytes: original 4-byte float and its inverted value
Return:  true (succeed)  ||  false (failed)
*/
bool Eeprom::write_float(uint16_t memory_address, float data) {
    std::array<uint8_t, EEPROM_PAYLOAD_SIZE_FLOAT> payload;
    
    payload[0] = static_cast<uint8_t>((memory_address >> BYTE_BITS) & 0xFF);
    payload[1] = static_cast<uint8_t>(memory_address & 0xFF);

    std::memcpy(&payload[2], &data, sizeof(float));

    for (std::size_t i = 0; i < sizeof(float); ++i) {
        payload[2 + sizeof(float) + i] = static_cast<uint8_t>(~payload[2 + i]);
    }

    if (!save_data(payload)) {
        std::cerr << "[ERROR-EEPROM] --> write_float: FAILED" << std::endl;
        return false;
    }

    return true;
}

/*
Read 8 bytes: value & ~value @ memory_address. 
Return:  validated_value (succeed)  ||  std::nullopt (failure)
*/
std::optional<float> Eeprom::read_float(uint16_t memory_address) const {
    std::array<uint8_t, EEPROM_BUFFER_SIZE_FLOAT> buffer{};
    
    if (!read_data(memory_address, buffer)) {
        std::cerr << "[ERROR-EEPROM] --> read_float: FAILED" << std::endl;
        return std::nullopt;
    }

    for (std::size_t i = 0; i < sizeof(float); ++i) {
        if (static_cast<uint8_t>(~buffer[i]) != buffer[i + sizeof(float)]) {
            std::cerr << "[ERROR-EEPROM] --> read_float: INVALID DATA" << std::endl;
            return std::nullopt;
        }
    }

    float value{0};
    std::memcpy(&value, buffer.data(), sizeof(float));

    return value;
}



/* ############## P R I V A T E ############## */


/*
Write --payload-- into eeporm. 
Return: true (succeed)  ||  false (failed)
*/
bool Eeprom::save_data(std::span<const uint8_t> payload) const {
    if (!poll_ack_with_timeout_ms(EEPROM_ACK_TIMEOUT_MS)) {
        std::cerr << "[ERROR-EEPROM] --> save_data: EEPROM busy" << std::endl;
        return false;
    }
    
    int bytes_written = i2c_write_blocking(
        I2C_PORT, 
        I2C_ADDR_EEPROM, 
        payload.data(), 
        payload.size(), 
        false
    );
    
    if (bytes_written != static_cast<int>(payload.size())) {
        std::cerr << "[ERROR-EEPROM] --> save_data: written wrong number of bytes" << std::endl;
        return false;
    }
    
    poll_ack_with_timeout_ms(EEPROM_ACK_TIMEOUT_MS);
    return true;
}

/*
Read data from --memory_address-- into --destination-- 
Return: true (succeed)  ||  false (failed)
*/
bool Eeprom::read_data(uint16_t memory_address, std::span<uint8_t> destination) const {
    if (!poll_ack_with_timeout_ms(EEPROM_ACK_TIMEOUT_MS)) {
        std::cerr << "[ERROR-EEPROM] --> read_data: EEPROM busy" << std::endl;
        return false;
    }

    const std::array<uint8_t, EEPROM_ADDR_BYTES> addr_bytes = {
        static_cast<uint8_t>((memory_address >> BYTE_BITS) & 0xFF),
        static_cast<uint8_t>(memory_address & 0xFF)
    };
    
    int written = i2c_write_blocking(
        I2C_PORT, 
        I2C_ADDR_EEPROM, 
        addr_bytes.data(), 
        addr_bytes.size(), 
        true
    );
    if (written != static_cast<int>(addr_bytes.size())) {
        std::cerr << "[ERROR-EEPROM] --> read_data: written wrong number of bytes" << std::endl;
        return false;
    }
    
    int read_bytes = i2c_read_blocking(
        I2C_PORT, 
        I2C_ADDR_EEPROM, 
        destination.data(), 
        destination.size(), 
        false
    );
    return read_bytes == static_cast<int>(destination.size());
}

/*
Poll ACK until:
    answer from eeprom == eeprom is free from previous write
    --timeout_ms-- has passed
Return:  true (ACK received)  ||  false (timeout)
*/
bool Eeprom::poll_ack_with_timeout_ms(uint32_t timeout_ms) const {
    uint8_t dummy{0};
    const uint32_t start_time = time_us_32();

    while ((time_us_32() - start_time) < timeout_ms*1000) {
        int result = i2c_read_timeout_us(
            I2C_PORT, 
            I2C_ADDR_EEPROM, 
            &dummy, 
            1, 
            false, 
            I2C_POLLING_TIMEOUT_US
        );

        if (result == 1) {
            return true;
        }
    }
    return false;
}

/*
Initialize I2C hardware
*/
void Eeprom::init_i2c_hardware() {
    i2c_init(I2C_PORT, I2C_FREQ);

    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}