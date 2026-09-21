#ifndef IOT_OOP_GARAGE_DOOR_INTERRUPTS_H
#define IOT_OOP_GARAGE_DOOR_INTERRUPTS_H

#include "pico/util/queue.h"

/* EVENT QUEUES */
extern queue_t sw0EventQ;
extern queue_t sw1EventQ;
extern queue_t sw2EventQ;
extern queue_t lmswEventQ1;
extern queue_t lmswEventQ2;
extern queue_t encAEventQ;
extern queue_t encBEventQ;

/* EVENT FUNCTIONS / CLASSES */
void commonIsr(uint gpio, uint32_t eventMask);
void initQueues();

#endif //IOT_OOP_GARAGE_DOOR_INTERRUPTS_H