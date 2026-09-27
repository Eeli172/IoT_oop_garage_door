#ifndef IOT_OOP_GARAGE_DOOR_TYPES_H
#define IOT_OOP_GARAGE_DOOR_TYPES_H

#include "lwip/apps/mqtt.h"
#include "../macros/macros.h"

// ENUMS 

typedef enum { CLOSED, IN_BETWEEN, OPEN } doorState_t;

typedef enum { PRE_CALIB, CALIBRATING, CALIBRATED } calibrationState_t;

typedef enum {UNSUBSCRIBE, SUBSCRIBE} mqttSubState_t;


// STRUCTS

typedef struct {
    mqtt_client_t *mqtt_client;
    ip_addr_t broker_ip;
    u_int16_t mqttPort;
    mqtt_connect_client_info_t ci;
} mqtt_information;

typedef struct {
    uint8_t payload_array[MAX_PAYLOAD_SIZE];
    uint payload_length;

    // Data to be stored inside the payload array
    uint8_t data_array[EEPROM_BYTES];
    uint data_length;
} payload_control_t;


#endif //IOT_OOP_GARAGE_DOOR_TYPES_H