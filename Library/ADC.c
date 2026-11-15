#include "ADC.h"
#include "ES28.h"

#define ADC_WAIT_LIMIT 100000U
#define ADC_LAST_CHANNEL 19U
#define ADC_LAST_WIDTH_CODE 3
#define ADC_CALIBRATION_MAX 0x7FU
#define ADC_12_BIT_MASK 0x0FFFU
#define ADC_REGULATOR_DELAY_MS 2
#define ADC_CALIBRATION_DELAY_MS 1

static int wait_for_set(volatile uint32_t *reg, uint32_t mask) {
    for (uint32_t n = 0; n < ADC_WAIT_LIMIT; ++n) {
        if ((*reg & mask) != 0U) {
            return 1;
        }
    }
    return 0;
}

static int wait_for_clear(volatile uint32_t *reg, uint32_t mask) {
    for (uint32_t n = 0; n < ADC_WAIT_LIMIT; ++n) {
        if ((*reg & mask) == 0U) {
            return 1;
        }
    }
    return 0;
}

void adc_setWidth(int widthCode) {
    if (widthCode < 0 || widthCode > ADC_LAST_WIDTH_CODE) {
        return;
    }
    if (!wait_for_clear(&ADC1->CR, ADC_CR_ADSTART)) {
        return;
    }

    // CFGR1 holds the resolution; the ADC must be off while changing it.
    uint32_t wasEnabled = ADC1->CR & ADC_CR_ADEN;
    if (wasEnabled != 0U) {
        ADC1->CR = ADC1->CR | ADC_CR_ADDIS;
        if (!wait_for_clear(&ADC1->CR, ADC_CR_ADEN)) {
            return;
        }
    }
    uint32_t config = ADC1->CFGR1;
    config = config & ~ADC_CFGR1_RES_Msk;
    config = config | ((uint32_t)widthCode << ADC_CFGR1_RES_Pos);
    ADC1->CFGR1 = config;
    if (wasEnabled != 0U) {
        ADC1->ISR = ADC_ISR_ADRDY;
        ADC1->CR = ADC1->CR | ADC_CR_ADEN;
        (void)wait_for_set(&ADC1->ISR, ADC_ISR_ADRDY);
    }
}

void adc_setChannel(unsigned int chNum) {
    // CHSELR has one selection bit for each channel, 0 through 19.
    if (chNum > ADC_LAST_CHANNEL) {
        return;
    }
    if (!wait_for_clear(&ADC1->CR, ADC_CR_ADSTART)) {
        return;
    }
    ADC1->ISR = ADC_ISR_CCRDY;
    ADC1->CHSELR = 1UL << chNum;
    (void)wait_for_set(&ADC1->ISR, ADC_ISR_CCRDY);
}

void adc_init(void) {
    // Turn on the GPIO and ADC clocks, then make A3 an analog input.
    RCC->IOPENR = RCC->IOPENR | RCC_IOPENR_GPIOBEN;
    /* GPIO analog mode is 11 in each pin's two mode bits. */
    GPIOB->MODER = GPIOB->MODER | GPIO_MODER_MODE1_Msk;
    RCC->APBENR2 = RCC->APBENR2 | RCC_APBENR2_ADCEN;

    // Start the ADC's internal regulator and calibrate before enabling it.
    ADC1->CR = ADC1->CR | ADC_CR_ADVREGEN;
    delay_ms(ADC_REGULATOR_DELAY_MS); // Longer than the required regulator startup time.
    ADC1->CR = ADC1->CR | ADC_CR_ADCAL;
    if (!wait_for_clear(&ADC1->CR, ADC_CR_ADCAL)) {
        return;
    }
    uint32_t calibration = (ADC1->CALFACT & ADC_CALFACT_CALFACT_Msk)
                         >> ADC_CALFACT_CALFACT_Pos;
    if (calibration < ADC_CALIBRATION_MAX) {
        ++calibration;
    }
    delay_ms(ADC_CALIBRATION_DELAY_MS);
    ADC1->SMPR = ADC1->SMPR | ADC_SMPR_SMP1_Msk;
    ADC1->CFGR1 = ADC1->CFGR1 & ~ADC_CFGR1_CHSELRMOD;
    // Rearm the ADC after long gaps between one-second readings.
    ADC1->CFGR2 = ADC1->CFGR2 | ADC_CFGR2_LFTRIG;
    adc_setWidth(0);
    adc_setChannel(18);
    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR = ADC1->CR | ADC_CR_ADEN;
    if (!wait_for_set(&ADC1->ISR, ADC_ISR_ADRDY)) {
        return;
    }
    // STM32C0 needs the adjusted calibration factor after ADC enable.
    uint32_t calibrationRegister = ADC1->CALFACT;
    calibrationRegister = calibrationRegister & ~ADC_CALFACT_CALFACT_Msk;
    calibrationRegister = calibrationRegister | (calibration << ADC_CALFACT_CALFACT_Pos);
    ADC1->CALFACT = calibrationRegister;
}

int adc_getValue(void) {
    if ((ADC1->CR & ADC_CR_ADEN) == 0U) {
        return -1;
    }
    if (!wait_for_clear(&ADC1->CR, ADC_CR_ADSTART)) {
        return -1;
    }
    ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
    // CR starts the conversion; ISR reports completion; DR holds the result.
    ADC1->CR = ADC1->CR | ADC_CR_ADSTART;
    if (!wait_for_set(&ADC1->ISR, ADC_ISR_EOC)) {
        return -1;
    }
    return (int)(ADC1->DR & ADC_12_BIT_MASK);
}
