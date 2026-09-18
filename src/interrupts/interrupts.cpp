#include "interrupts.h"
#include "../macros/macros.h"
// queuet + isr funktiot

void commonIsr(const uint gpio, uint32_t eventMask)
{
    constexpr int value = true;
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