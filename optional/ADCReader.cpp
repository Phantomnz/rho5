#include "ADCReader.hpp"
#include <avr/io.h> // For ADC registers and bit definitions

ADCReader::ADCReader() {
    // Initialize the ADC
    ADMUX = 0; // setting to 0 to select channel 0 by default, AVcc as reference
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1); // Enable ADC, prescaler of 64
    DDRA &= ~_BV(PA0);
}

uint16_t ADCReader::readADC(uint8_t channel) {
    
    // Select the channel
    // We clear the bottom 5 bits (MUX4:0) and set them to 'channel'.
    // 0xE0 is 11100000 in binary.
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);

    // Start the conversion
    ADCSRA |= _BV(ADSC);

    // Wait for conversion to complete
    while (ADCSRA & _BV(ADSC));

    // Return the ADC value
    return ADC;
}