#ifndef IOT_OOP_GARAGE_DOOR_TYPES_H
#define IOT_OOP_GARAGE_DOOR_TYPES_H

#include "lwip/apps/mqtt.h"

// ENUMS 

typedef enum { CLOSED, IN_BETWEEN, OPEN } doorState_t;

typedef enum { PRE_CALIB, CALIBRATING, CALIBRATED } stateMachine_t;

typedef enum {UNSUBSCRIBE, SUBSCRIBE} mqttSubState_t;


// STRUCTS

typedef struct {
    mqtt_client_t *mqtt_client;
    ip_addr_t broker_ip;
    u_int16_t mqttPort;
    mqtt_connect_client_info_t ci;
} mqtt_information;


#endif //IOT_OOP_GARAGE_DOOR_TYPES_H