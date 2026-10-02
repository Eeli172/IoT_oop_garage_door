#ifndef IOT_OOP_GARAGE_DOOR_EEPROM_H
#define IOT_OOP_GARAGE_DOOR_EEPROM_H

#include <sys/types.h>


class Eeprom {
public:
    Eeprom();
    int write(uint16_t addr, uint data);
    int read(uint16_t addr, int* value);
    
private:
    void init_i2c_hw();
    uint package_data(const uint8_t* data_array, uint data_array_length, uint8_t* payload_array, uint16_t memory_address);
    uint save_data(const uint8_t* payload, uint len);
    uint read_data(uint8_t* data, uint len, const uint8_t* addr);
    bool validate_state(const uint8_t* array, int array_length);
    uint count_bytes(uint value);
    void ack_polling_with_timeout(uint timeout_us);
};



#endif //IOT_OOP_GARAGE_DOOR_EEPROM_H