#include "interrupts.h"
#include "../macros/macros.h"
#include "hardware/gpio.h"

/* ISR QUEUES */
queue_t sw0EventQ;
queue_t sw1EventQ;
queue_t sw2EventQ;
queue_t lmswEventQ;
queue_t encEventQ;

void commonIsr(const uint gpio, const uint32_t eventMask)
{
    gpio_set_irq_enabled(gpio, eventMask, false);

    static absolute_time_t previous;

    if (absolute_time_diff_us(previous, get_absolute_time()) > 500000) {// 1000 ms

        previous = get_absolute_time();

        if (gpio == LMSW_1 || gpio == LMSW_2) {
            queue_try_add(&lmswEventQ, &gpio);
        }

        else if (gpio == SW0) {
            queue_try_add(&sw0EventQ, &gpio);
        }

        else if (gpio == SW1) {
            queue_try_add(&sw1EventQ, &gpio);
        }

        else if (gpio == SW2) {
            queue_try_add(&sw2EventQ, &gpio);
        }

        else if (gpio == ENCODER_A || gpio == ENCODER_B) {
            queue_try_add(&encEventQ, &gpio);
        }
    }
}

void initQueues()
{
    queue_init(&sw0EventQ, sizeof(uint), 10);
    queue_init(&sw1EventQ, sizeof(uint), 10);
    queue_init(&sw2EventQ, sizeof(uint), 10);
    queue_init(&lmswEventQ, sizeof(uint), 10);
    queue_init(&encEventQ, sizeof(uint), 10);
}