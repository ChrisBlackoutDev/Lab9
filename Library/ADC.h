#ifndef LAB5_ADC_H
#define LAB5_ADC_H

/* Configures PB1/A3 as an analog input; starts at channel 18, 12 bits. */
void adc_init(void);

/* Starts one conversion on the potentiometer channel. Returns the raw unsigned ADC
   code, or -1 if the ADC is disabled or a hardware wait times out. */
int adc_getValue(void);

#endif
