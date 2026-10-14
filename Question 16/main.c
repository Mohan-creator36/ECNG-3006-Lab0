#include <sam.h>
#include <stdint.h>

#define LED_MASK       (1UL << 10)
#define ADC_THRESHOLD  2048U

/* Available for inspection with a debugger. */
volatile uint16_t adc_value = 0U;

static void adc_wait_sync(void)
{
    while (ADC->STATUS.bit.SYNCBUSY)
    {
    }
}

static void adc_init(void)
{
    /* Enable the internal 8 MHz oscillator. */
    SYSCTRL->OSC8M.bit.ONDEMAND = 0;
    SYSCTRL->OSC8M.bit.PRESC = 0;
    SYSCTRL->OSC8M.bit.ENABLE = 1;

    while (!SYSCTRL->PCLKSR.bit.OSC8MRDY)
    {
    }

    /* Configure generic clock generator 1. */
    GCLK->GENDIV.reg =
        GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);

    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_ID(1)
        | GCLK_GENCTRL_SRC_OSC8M
        | GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    /* Enable the ADC bus and peripheral clocks. */
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_ADC
        | GCLK_CLKCTRL_GEN_GCLK1
        | GCLK_CLKCTRL_CLKEN;

    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    /* A0 = PA02. Peripheral function B selects analog input. */
    PORT->Group[0].DIRCLR.reg = (1UL << 2);
    PORT->Group[0].PMUX[1].bit.PMUXE = 1;
    PORT->Group[0].PINCFG[2].reg = PORT_PINCFG_PMUXEN;

    /* Disable and reset the ADC. */
    ADC->CTRLA.reg = 0;
    adc_wait_sync();

    ADC->CTRLA.reg = ADC_CTRLA_SWRST;

    while (ADC->STATUS.bit.SYNCBUSY ||
           ADC->CTRLA.bit.SWRST)
    {
    }

    /* Load SAMD21 factory calibration values. */
    const uint32_t calibration_low =
        *((const uint32_t *)0x00806020UL);

    const uint32_t calibration_high =
        *((const uint32_t *)0x00806024UL);

    const uint32_t linearity =
        ((calibration_low >> 27) & 0x1FUL)
        | ((calibration_high & 0x07UL) << 5);

    const uint32_t bias =
        (calibration_high >> 3) & 0x07UL;

    ADC->CALIB.reg =
        ADC_CALIB_LINEARITY_CAL(linearity)
        | ADC_CALIB_BIAS_CAL(bias);

    /* Reference = supply/2, with input gain = 1/2.
       Together these support a 0-to-supply input range. */
    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
    adc_wait_sync();

    ADC->CTRLB.reg =
        ADC_CTRLB_PRESCALER_DIV32
        | ADC_CTRLB_RESSEL_12BIT;
    adc_wait_sync();

    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(15);
    adc_wait_sync();

    ADC->INPUTCTRL.reg =
        ADC_INPUTCTRL_MUXPOS_PIN0
        | ADC_INPUTCTRL_MUXNEG_GND
        | ADC_INPUTCTRL_GAIN_DIV2;
    adc_wait_sync();

    ADC->CTRLA.reg = ADC_CTRLA_ENABLE;
    adc_wait_sync();
}

static uint16_t adc_read(void)
{
    /* Clear the previous conversion-complete flag. */
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    ADC->SWTRIG.reg = ADC_SWTRIG_START;
    adc_wait_sync();

    while (!ADC->INTFLAG.bit.RESRDY)
    {
    }

    return (uint16_t)ADC->RESULT.reg;
}

int main(void)
{
    PORT->Group[0].OUTCLR.reg = LED_MASK;
    PORT->Group[0].DIRSET.reg = LED_MASK;

    adc_init();

    /* Discard the first conversion after reference setup. */
    (void)adc_read();

    while (1)
    {
        adc_value = adc_read();

        if (adc_value >= ADC_THRESHOLD)
        {
            PORT->Group[0].OUTSET.reg = LED_MASK;
        }
        else
        {
            PORT->Group[0].OUTCLR.reg = LED_MASK;
        }
    }
}
