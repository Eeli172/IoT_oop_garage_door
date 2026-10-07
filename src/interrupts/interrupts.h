#ifndef IOT_OOP_GARAGE_DOOR_INTERRUPTS_H
#define IOT_OOP_GARAGE_DOOR_INTERRUPTS_H

#include "pico/util/queue.h"

/* EVENT QUEUES */
extern queue_t sw1EventQ;
extern queue_t lmswMtrEventQ;
extern queue_t lmswEncEventQ;
extern queue_t encEventQ;

/* EVENT FUNCTIONS / CLASSES */
void commonIsr(uint gpio, uint32_t eventMask);
void initInterruptQueues();


#endif //IOT_OOP_GARAGE_DOOR_INTERRUPTS_H