#ifndef IOT_OOP_GARAGE_DOOR_TYPES_H
#define IOT_OOP_GARAGE_DOOR_TYPES_H


typedef enum { CLOSED, IN_BETWEEN, OPEN } doorState_t;

typedef enum { PRE_CALIB, CALIBRATING, CALIBRATED } stateMachine_t;

typedef enum {UNSUBSCRIBE, SUBSCRIBE} mqttSubState_t;

#endif //IOT_OOP_GARAGE_DOOR_TYPES_H
