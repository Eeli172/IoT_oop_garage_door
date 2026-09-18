#include "interrupts.h"
#include "../macros/macros.h"
#include <iostream>

/* ISR QUEUES */
queue_t sw0EventQ;
queue_t sw1EventQ;
queue_t sw2EventQ;

void commonIsr(const uint gpio, uint32_t eventMask)
{
    const bool value = true;

    if (gpio == SW0)
    {
        queue_try_add(&sw0EventQ, &value);
    }

    else if (gpio == SW1)
    {
        queue_try_add(&sw1EventQ, &value);
    }

    else if (gpio == SW2)
    {
        queue_try_add(&sw2EventQ, &value);
    }
}

void initQueues()
{
    queue_init(&sw0EventQ, sizeof(bool), 10);
    queue_init(&sw1EventQ, sizeof(bool), 10);
    queue_init(&sw2EventQ, sizeof(bool), 10);
}