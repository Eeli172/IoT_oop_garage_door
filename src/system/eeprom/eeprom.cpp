#include "eeprom.h"
#include "hardware/i2c.h"
#include "types/types.h"
#include <hardware/gpio.h>
#include <hardware/structs/io_bank0.h>
#include <iostream>


Eeprom::Eeprom() {
    init_i2c_hw();
}


int Eeprom::write(const uint16_t address, const uint data)
{
    payload_control_t payload;
    payload.data_length = count_bytes(data);
    payload.data_length *= 2; // original and inverted values
    printf("WR: 1\n");
    if (payload.data_length == EEPROM_BYTES)
    {
    printf("WR: 2\n");
        payload.data_array[0] = (uint8_t)data;
        payload.data_array[1] = ~payload.data_array[0];
    }
    else
    {
        std::cerr << "[ERROR] --> EEPROM: wrong amount of bytes." << std::endl;
        return -1;
    }
    
    package_data(payload.data_array, payload.data_length, payload.payload_array, address);
    printf("WR: 3\n");
    payload.payload_length = payload.data_length + ADDRESS_BYTES;
    
    if (save_data(payload.payload_array, payload.payload_length) != payload.payload_length)
    {
    printf("WR: 4\n");
        printf("[ERROR] --> Program Eeprom save failed at MEM_ADD: %X\n", address);
        return -1;
    }
    printf("WR: 5\n");
    
    return 0;
}
int Eeprom::read(uint16_t addr, int* value) {
    const uint8_t memory_address[ADDRESS_BYTES] = {
        static_cast<uint8_t>(addr >> 8 & 0xFF), 
        static_cast<uint8_t>(addr & 0xFF)
    };
    uint8_t data_array[EEPROM_BYTES];

    read_data(data_array, EEPROM_BYTES, memory_address);


    if (validate_state(data_array, EEPROM_BYTES))
    {
        *value = (int) data_array[0];
        return 0;
    }

    else { printf("Previous setting not found at EEPROM\n"); }

    return 1;
}


void Eeprom::init_i2c_hw() {
    i2c_init(I2C_PORT, I2C_FREQ);

    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}

uint Eeprom::package_data(const uint8_t* data_array, const uint data_array_length, uint8_t* payload_array,
                  const uint16_t memory_address)
{
    payload_array[0] = memory_address >> 8 & 0xFF;
    payload_array[1] = memory_address & 0xFF;

    for (uint i = 0; i < data_array_length; i++)
    {
        payload_array[i + 2] = data_array[i];
    }

    return 0;
}
uint Eeprom::save_data(const uint8_t* payload, const uint len)
{
    printf("SD: 1\n");

    const uint bytes_written = i2c_write_blocking(I2C_PORT, I2C_ADDR_EEPROM, payload, len, false);
    printf("SD: 2\n");
    ack_polling_with_timeout(EEPROM_ACK_TIMEOUT_US);
    printf("SD: 3\n");

    return bytes_written;
}
uint Eeprom::read_data(uint8_t* data, const uint len, const uint8_t* addr)
{
    // SETS THE POINTER INSIDE THE EEPROM TO 16-BIT ADDRESS 0xFFFA (6th last)
    i2c_write_blocking(I2C_PORT, I2C_ADDR_EEPROM, addr, 2, true);
    
    // READS FROM THE MEM ADDRESS POINTED AT
    return i2c_read_blocking(I2C_PORT, I2C_ADDR_EEPROM, data, len, false);
}
bool Eeprom::validate_state(const uint8_t* array, const int array_length)
{
    for (int i = 0; i < array_length; i += 2)
    {
        // Inverts the uninverted value and compares it to the inverted value
        if ((uint8_t)~array[i] != array[i + 1])
        {
            return false;
        }
    }
    return true;
}
uint Eeprom::count_bytes(uint value)
{
    uint bytes = 1;
    while (value > 0xFF)
    {
        value = value >> 8;
        
        bytes++;
    }
    return bytes;
}
void Eeprom::ack_polling_with_timeout(uint timeout_us)
{
    uint8_t dummy = 0;
    int read_bytes = 1;
    uint32_t start_time_us = time_us_32();

    while ((time_us_32() - start_time_us) < timeout_us)
    {
        int result = i2c_read_timeout_us(I2C_PORT, I2C_ADDR_EEPROM, 
            &dummy, read_bytes, false, I2C_POLLING_TIMEOUT_US);

        if (result == read_bytes) 
        {
            // printf("EEPROM_ACK_POLL_TIME_US: %d\n", time_us_32()-start_time_us);
            return;
        }
    }
}