#include <sam.h>
#include <stdint.h>

#define LED_MASK    (1UL << 10)
#define DELAY_COUNT 1000000UL

void delay_loop(void)
{
    for (volatile uint32_t count = 0;
         count < DELAY_COUNT;
         count++)
    {
    }
}

int main(void)
{
    /* Initialise the output LOW before enabling it. */
    PORT->Group[0].OUTCLR.reg = LED_MASK;
    PORT->Group[0].DIRSET.reg = LED_MASK;

    while (1)
    {
        /* Turn the LED on, then wait. */
        PORT->Group[0].OUTSET.reg = LED_MASK;
        delay_loop();

        /* Turn the LED off, then wait. */
        PORT->Group[0].OUTCLR.reg = LED_MASK;
        delay_loop();
    }
}
