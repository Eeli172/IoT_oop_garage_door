#ifndef IOT_OOP_GARAGE_DOOR_TYPES_H
#define IOT_OOP_GARAGE_DOOR_TYPES_H

#include "lwip/apps/mqtt.h"

// ENUMS 

typedef enum { CLOSED, IN_BETWEEN, OPEN } doorState_t;

typedef enum { NOT_CALIBRATED, CALIBRATING, CALIBRATED } calibrationState_t;

typedef enum { UNSUBSCRIBE, SUBSCRIBE, PUBLISH } mqttSubState_t;


// STRUCTS

typedef struct {
    mqtt_client_t *mqtt_client;
    ip_addr_t broker_ip;
    u_int16_t mqttPort;
    mqtt_connect_client_info_t ci;
} mqttInformation_t;

typedef struct {
    float motorStepsPerEncoder;
    bool valid;
} MovementCalibration_t;




#endif //IOT_OOP_GARAGE_DOOR_TYPES_H