/*
 * ADC input for the A3/PB1 potentiometer on channel 18.
 * The conversion and calibration sequence is adapted from the Lab 8 ADC library.
 */
#include "ADC.h"
#include "ES28.h"

#define ADC_WAIT_LIMIT 100000U
#define ADC_CALIBRATION_MAX 0x7FU
#define ADC_12_BIT_MASK 0x0FFFU
#define ADC_REGULATOR_DELAY_MS 2
#define ADC_CALIBRATION_DELAY_MS 1

static int WaitForSet(volatile uint32_t *reg, uint32_t mask) {
    for (uint32_t n = 0; n < ADC_WAIT_LIMIT; ++n) {
        if ((*reg & mask) != 0U) {
            return 1;
        }
    }
    return 0;
}

static int WaitForClear(volatile uint32_t *reg, uint32_t mask) {
    for (uint32_t n = 0; n < ADC_WAIT_LIMIT; ++n) {
        if ((*reg & mask) == 0U) {
            return 1;
        }
    }
    return 0;
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
    if (!WaitForClear(&ADC1->CR, ADC_CR_ADCAL)) {
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
    // Rearm the ADC after idle gaps.
    ADC1->CFGR2 = ADC1->CFGR2 | ADC_CFGR2_LFTRIG;
    /* Select 12-bit channel 18 for the A3/PB1 potentiometer. */
    ADC1->CFGR1 = ADC1->CFGR1 & ~ADC_CFGR1_RES_Msk;
    ADC1->ISR = ADC_ISR_CCRDY;
    ADC1->CHSELR = ADC_CHSELR_CHSEL18;
    if (!WaitForSet(&ADC1->ISR, ADC_ISR_CCRDY)) {
        return;
    }
    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR = ADC1->CR | ADC_CR_ADEN;
    if (!WaitForSet(&ADC1->ISR, ADC_ISR_ADRDY)) {
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
    if (!WaitForClear(&ADC1->CR, ADC_CR_ADSTART)) {
        return -1;
    }
    ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
    // CR starts the conversion; ISR reports completion; DR holds the result.
    ADC1->CR = ADC1->CR | ADC_CR_ADSTART;
    if (!WaitForSet(&ADC1->ISR, ADC_ISR_EOC)) {
        return -1;
    }
    return (int)(ADC1->DR & ADC_12_BIT_MASK);
}
