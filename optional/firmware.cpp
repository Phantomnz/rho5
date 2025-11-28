#include "config.hpp"
#include "ADCReader.hpp"
#include "PIDController.hpp"
#include "AVRSerial.hpp"
#include "PWMTimer.hpp"
#include <util/delay.h>
#include <avr/io.h>

PWMTimer g_timer;
ADCReader g_adc;
PIDController g_pid(INITIAL_KP, INITIAL_KI, INITIAL_KD); // Initial PID gains
AVRSerial g_serial;

int main(void) {
    uint16_t current_setpoint = INITIAL_SETPOINT;
    uint16_t measured_value = 0;
    uint8_t pwm_output = 0;
    
    // A counter to slow down the sending of data
    int send_counter = 0;

    g_serial.sendData(current_setpoint, measured_value, pwm_output); 

    for(;;) {
        DDRB |= _BV(PB7);  // Set PB7 as output for debug LED
        // ALWAYS check for new commands first
        g_serial.processIncomingData(g_pid, current_setpoint); 

        // Run the Control Loop (Fast)
        measured_value = g_adc.readADC(0);
        pwm_output = g_pid.update(current_setpoint, measured_value);
        g_timer.setDutyCycle(pwm_output);

        // Talk to PC 
        send_counter++;
        if (send_counter >= 10) {
            g_serial.sendData(current_setpoint, measured_value, pwm_output);
            send_counter = 0;
        }

        // Smart Sleep
        // Instead of sleeping for 10ms straight, we sleep 10 x 1ms
        // and check for serial commands in between.
        for (int i = 0; i < LOOP_TIME_MS; i++) {
            _delay_ms(1);
            g_serial.processIncomingData(g_pid, current_setpoint);
        }
    }
}