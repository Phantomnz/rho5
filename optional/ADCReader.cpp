#include "ADCReader.hpp"
#include <avr/io.h> 

ADCReader::ADCReader() {
    // 1. Select AVcc as reference
    ADMUX = _BV(REFS0); 
    // 2. Enable ADC, Prescaler /64
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1); 
    
    // 3. Ensure PA2 is Input (Changed from PA0)
    DDRA &= ~_BV(PA2); 
}

uint16_t ADCReader::readADC(uint8_t channel) {
    // Mask bottom 5 bits and set channel
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
    ADCSRA |= _BV(ADSC);
    while (ADCSRA & _BV(ADSC));
    return ADC;
}