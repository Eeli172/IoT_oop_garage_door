#include "interrupts.h"
#include "../macros/macros.h"
#include "hardware/gpio.h"

queue_t sw1EventQ;
queue_t lmswMtrEventQ;
queue_t lmswEncEventQ;
queue_t encEventQ;


void commonIsr(const uint gpio, const uint32_t eventMask)
{
    static constexpr bool value = true;
    static absolute_time_t previous;

    if (gpio == PIN_ENCODER_A || gpio == PIN_ENCODER_B) {
        queue_try_add(&encEventQ, &value);
    }
    else {
        
        if (absolute_time_diff_us(previous, get_absolute_time()) > INTERRUPT_TIMEOUT_MS * 1000) {
            if (eventMask == GPIO_IRQ_EDGE_FALL)
            {
                switch (gpio)
                {
                    case PIN_LMSW_ENC:
                    queue_try_add(&lmswEncEventQ, &value);
                    break;

                    case PIN_LMSW_MTR:
                    queue_try_add(&lmswMtrEventQ, &value);
                    break;
                    
                    case PIN_SW1:
                    queue_try_add(&sw1EventQ, &value);
                    break;
                }
            }
        }
    previous = get_absolute_time();
    }
}

void initQueues()
{
    queue_init(&sw1EventQ, sizeof(bool), 10);
    queue_init(&lmswMtrEventQ, sizeof(bool), 10);
    queue_init(&lmswEncEventQ, sizeof(bool), 10);
    queue_init(&encEventQ, sizeof(bool), 10);
}