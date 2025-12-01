#include "config.hpp"
#include "AVRSerial.hpp"
#include "PWMTimer.hpp"
#include <util/delay.h>
#include <avr/io.h>

PWMTimer g_timer; // Sets up PD4
AVRSerial g_serial; // Sets up UART1

// The Buffer
#define WAVE_SIZE 256
uint8_t g_waveform[WAVE_SIZE];

int main(void) {
    // Initialize with a sawtooth pattern so we see something by default
    for(int i=0; i<WAVE_SIZE; i++) {
        g_waveform[i] = i; 
    }

    uint8_t play_index = 0;

    for(;;) {
        // 1. CHECK FOR INCOMING DATA
        // We look directly at the register because AVRSerial.cpp is designed for PID commands
        if (UCSR1A & _BV(RXC1)) {
            char cmd = UDR1;
            
            // If we receive the 'w' header, enter Download Mode
            if (cmd == 'w') {
                // Turn on LED to indicate receiving
                DDRB |= _BV(PB7);
                PORTB |= _BV(PB7);

                for(int i=0; i<WAVE_SIZE; i++) {
                    // Wait for byte
                    while (!(UCSR1A & _BV(RXC1)));
                    g_waveform[i] = UDR1;
                }
                
                // Turn off LED
                PORTB &= ~_BV(PB7);
            }
        }

        // 2. PLAYBACK
        // Output the current sample
        g_timer.setDutyCycle(g_waveform[play_index]);

        // Move to next sample
        play_index++; 
        // uint8_t automatically wraps from 255 to 0, so no "if" needed!

        // 3. SPEED CONTROL
        // This delay determines the frequency of the wave.
        // 256 samples * 10000us = ~2.56s period (~0.39Hz wave)
        _delay_us(10000); 
    }
}