#include "interrupts.h"
#include "../macros/macros.h"
#include <iostream>

#include "hardware/gpio.h"

/* ISR QUEUES */
queue_t sw0EventQ;
queue_t sw1EventQ;
queue_t sw2EventQ;
queue_t lmswEventQ;

void commonIsr(const uint gpio, uint32_t eventMask)
{
    constexpr bool value = true;

    switch (gpio)
    {
        case SW0:
            queue_try_add(&sw0EventQ, &value);
        break;

        case SW1:
        queue_try_add(&sw1EventQ, &value);
        break;

        case SW2:
        queue_try_add(&sw2EventQ, &value);
        break;

        case LMSW_1:
        queue_try_add(&lmswEventQ, &value);
        break;

        case LMSW_2:
        queue_try_add(&lmswEventQ, &value);
        break;
    }
}

void initQueues()
{
    queue_init(&sw0EventQ, sizeof(bool), 10);
    queue_init(&sw1EventQ, sizeof(bool), 10);
    queue_init(&sw2EventQ, sizeof(bool), 10);
    queue_init(&lmswEventQ, sizeof(bool), 10);
}