#ifndef IOT_OOP_GARAGE_DOOR_INTERRUPTS_H
#define IOT_OOP_GARAGE_DOOR_INTERRUPTS_H

/* CUSTOM HEADERS */
#include "pico/util/queue.h"

// ========================================================================
// INTERRUPT EVENT QUEUES
// ========================================================================

extern queue_t sw1EventQ;
extern queue_t lmswMtrEventQ;
extern queue_t lmswEncEventQ;
extern queue_t encEventQ;

// ========================================================================
// INTERRUPT HANDLING
// ========================================================================

/**
 * @brief RP2040 shares a common ISR across all the IO devices.
 * Handles the incoming IRQs from all the IO devices.
 * @param gpio Determines the pin corresponding to an IRQ.
 * @param eventMask Determines the edge (RISING, FALLING) or a static level (HIGH, LOW).
 */
void commonIsr(uint gpio, uint32_t eventMask);

/**
 * @brief Initializes all used global interrupt queues.
 */
void initInterruptQueues();


#endif //IOT_OOP_GARAGE_DOOR_INTERRUPTS_H