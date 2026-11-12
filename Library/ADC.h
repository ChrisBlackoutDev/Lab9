#ifndef LAB5_ADC_H
#define LAB5_ADC_H

/* Configures PA0/A0, PA1/A1 and PB1/A3 as analog inputs; starts at channel 0, 12 bits. */
void adc_init(void);

/* Selects ADC channel 0..19 for the next conversion; invalid channels are ignored.
   The caller configures any external GPIO other than PA0/PA1/PB1 as analog. */
void adc_setChannel(unsigned int chNum);

/* Resolution codes: 0=12 bits, 1=10 bits, 2=8 bits, 3=6 bits.
   Invalid codes are ignored. Change width only between conversions. */
void adc_setWidth(int widthCode);

/* Starts one conversion on the selected channel. Returns the raw unsigned ADC
   code, or -1 if the ADC is disabled or a hardware wait times out. */
int adc_getValue(void);

#endif
