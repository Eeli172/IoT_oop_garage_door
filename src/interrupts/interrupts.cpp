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
    static absolute_time_t debounceTimers[NUM_BANK0_GPIOS] = {}; // One per GPIO pin

    if (gpio == PIN_ENCODER_A)
    {
        queue_try_add(&encEventQ, &value);
        return;
    }
    else // UPDATE TIMER ON BOTH EDGES
    {
        // if (switch pressed) || (switch pressed but debounce were too fast with the next edge)
        if ( (eventMask == GPIO_IRQ_EDGE_FALL) || (eventMask == GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE) )
        {
            // Check debounce for this specific pin
            if (absolute_time_diff_us(debounceTimers[gpio], get_absolute_time()) > INTERRUPT_TIMEOUT_MS * 1000)
            {
                switch (gpio)
                {
                    case PIN_LMSW_ENC_CLOSED:
                    queue_try_add(&lmswEncEventQ, &value);
                    break;
                    
                    case PIN_LMSW_MTR_OPEN:
                    queue_try_add(&lmswMtrEventQ, &value);
                    break;
                    
                    case PIN_SW1:
                    queue_try_add(&sw1EventQ, &value);
                    break;
                }
                
            }
        }
        debounceTimers[gpio] = get_absolute_time(); // Update only this pin's timer
    }
}

void initQueues()
{
    queue_init(&sw1EventQ, sizeof(bool), 10);
    queue_init(&lmswMtrEventQ, sizeof(bool), 10);
    queue_init(&lmswEncEventQ, sizeof(bool), 10);
    queue_init(&encEventQ, sizeof(bool), 100);
}