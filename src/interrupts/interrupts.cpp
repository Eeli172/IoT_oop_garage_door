#include "interrupts.h"
#include "../macros/macros.h"

/* ISR QUEUES */
queue_t sw0EventQ;
queue_t sw1EventQ;
queue_t sw2EventQ;
queue_t lmswEventQ1;
queue_t lmswEventQ2;
queue_t encAEventQ;
queue_t encBEventQ;

void commonIsr(const uint gpio, uint32_t eventMask)
{
    constexpr bool value = true;
    static absolute_time_t previous;

    if (absolute_time_diff_us(previous, get_absolute_time()) > 1000000) {// 1000 ms

        previous = get_absolute_time();
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
            queue_try_add(&lmswEventQ1, &value);
            break;

            case LMSW_2:
            queue_try_add(&lmswEventQ2, &value);
            break;

            case ENCODER_A:
            queue_try_add(&encAEventQ, &value);
            break;

            case ENCODER_B:
            queue_try_add(&encBEventQ, &value);
            break;
        }
    }
}

void initQueues()
{
    queue_init(&sw0EventQ, sizeof(bool), 10);
    queue_init(&sw1EventQ, sizeof(bool), 10);
    queue_init(&sw2EventQ, sizeof(bool), 10);
    queue_init(&lmswEventQ1, sizeof(bool), 10);
    queue_init(&lmswEventQ2, sizeof(bool), 10);
    queue_init(&encAEventQ, sizeof(bool), 10);
    queue_init(&encBEventQ, sizeof(bool), 10);
}