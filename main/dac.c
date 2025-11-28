#include <avr/io.h>
#include <util/delay.h>
#include "debug.h"

/*
 PIN | PORT | Description
 PD5 |  D   | Tone oscillator output
*/

/*
Register  | Description
 TCCR1A   | Timer/Counter1 Control Register A
 TCCR1B   | Timer/Counter1 Control Register B
 OCR1A    | Output Compare Register 1 A

Bits in TCCR1A:
 WGM10    | Waveform Generation Mode bit 0
 COM1A1   | Compare Output Mode for Channel A bit 1

Bits in TCCR1B:
 CS11     | Clock Select bit 1
 WGM12    | Waveform Generation Mode bit 2

*/

void init_pwm_dac(void);

int main(void) {
    init_debug_uart0();
    init_pwm_dac();
    uint8_t i = 0;
    for (;;) {
        char command = ugetchar0(stdin);
        if (command == 'u' && i < 255) {
            i++;
        } else if (command == 'd' && i > 0) {
            i--;
        }
        OCR1A = i;  /* set frequency */
        printf("Duty cycle: %u Hz\n", i);
        
    }
    

}

void init_pwm_dac(void)
{
    DDRD = _BV(PD5);
    TCCR1A |= _BV(WGM10) | _BV(COM1A1);  /* Toggle OC1A on compare match */
	TCCR1B |= _BV(CS11) | _BV(WGM12);
}



