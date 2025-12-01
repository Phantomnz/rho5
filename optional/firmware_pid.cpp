#include "config.hpp"
#include "ADCReader.hpp"
#include "PIDController.hpp"
#include "AVRSerial.hpp"
#include "PWMTimer.hpp"
#include <util/delay.h>
#include <avr/io.h>

PWMTimer g_timer;
ADCReader g_adc;
PIDController g_pid(INITIAL_KP, INITIAL_KI, INITIAL_KD); 
AVRSerial g_serial;

int main(void) {
    uint16_t current_setpoint = 0;
    uint16_t measured_value = 0;
    
    // STRICTLY 8-BIT
    uint8_t pwm_output = 0;
    
    int send_counter = 0;

    for(;;) {
        g_serial.processIncomingData(g_pid, current_setpoint); 

        measured_value = g_adc.readADC(2); // Keep PA2
        
        // Update PID
        pwm_output = g_pid.update(current_setpoint, measured_value);
        
        // Apply to hardware
        g_timer.setDutyCycle(pwm_output);

        // Send Data
        send_counter++;
        if (send_counter >= 10) {
            g_serial.sendData(current_setpoint, measured_value, pwm_output);
            send_counter = 0;
        }

        for (int i = 0; i < LOOP_TIME_MS; i++) {
            _delay_ms(1);
            g_serial.processIncomingData(g_pid, current_setpoint);
        }
    }
}