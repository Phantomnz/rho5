#include <avr/io.h>
#include <util/delay.h>
#include "debug.h"

void init_adc(void)
{
	DDRA &= ~_BV(PA0);
	//       Enable ADC |   /64 clock prescaler
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
	// Select PA3 as ADC single ended input channel
	ADMUX |= 0;
}

uint16_t read_adc(void)
{
    ADCSRA |= _BV(ADSC);
    // Busy loop while ADC Start conversion bit is set
    while ((ADCSRA & _BV(ADSC)));

    // ADSC is 0 now so ADC may be read completely.
	return ADC;
}

int main(void)
{
	uint16_t result;
    double voltage;
	
	init_debug_uart0();
	init_adc();
	
	for (;;) 
	{
        result = read_adc();
        voltage = (result / 1024.0) * 3.3;
		printf("%4d : %6.5fV\n", result, voltage);
		_delay_ms(1000);
	}
}