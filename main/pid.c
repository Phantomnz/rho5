/*
 * - Actuator (Output): PWM on PD5 (via OCR1A)
 * - Sensor (Input):   ADC on PA0 (reads the output voltage)
 * - Debug:            UART Serial to PC
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "debug.h"

//CONTROLLER TUNING & DEFINITIONS


// --- Setpoint ---
// Our target voltage. (1.65V / 3.3V) * 1023 = 511.5
#define SETPOINT 512

// Gains 
#define KP 0.5   // Proportional Gain (How hard to push)
#define KI 0.1   // Integral Gain (How fast to fix steady-state error)
#define KD 0.05  // Derivative Gain (How much to resist change)

// Time 
// The control loop speed. (10ms = 0.01s)
#define LOOP_TIME_MS 10
#define LOOP_TIME_S 0.01

// Controller Internals
#define PWM_MIN 0
#define PWM_MAX 255
// The "neutral" PWM value we add our PID output to.
// 512 (Setpoint) / 1023 (ADC max) * 255 (PWM max) = ~128
#define PWM_BIAS 128 

// --- Anti-Windup Limits ---
// Clamps the integral_sum to prevent it from growing forever.
// This is essential for good performance when the load changes.
#define INTEGRAL_MAX 50.0
#define INTEGRAL_MIN -50.0

// PID state variables
double integral_sum = 0.0;
double last_error = 0.0;


// hardware functions
void init_pwm_dac(void) {
    // Sets up Timer1 for 8-bit Fast PWM (Mode 5)
    // Output is on PD5 (OC1A)
    // PWM frequency = 12MHz / (8 * 256) = ~5.86 kHz
    DDRD |= _BV(PD5);
    TCCR1A = _BV(COM1A1) | _BV(WGM10); // Non-inverting PWM
    TCCR1B = _BV(CS11) | _BV(WGM12);   // Prescaler=8, Mode 5
}

void init_adc(void) {
    // Sets up ADC to read from PA0
    DDRA &= ~_BV(PA0); // Set PA0 to input
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1); // Enable ADC, /64 prescaler
    ADMUX = 0; // Use channel PA0
}

uint16_t read_adc(void) {
    // Start a single conversion
    ADCSRA |= _BV(ADSC);
    // Wait for it to finish
    while (ADCSRA & _BV(ADSC));
    // Return the 10-bit result
    return ADC;
}


int main(void) {
    uint16_t measured_value;
    double error;
    double p_out, i_out, d_out, output;

    // 1. Initialise all hardware
    init_debug_uart0();
    init_adc();
    init_pwm_dac();
    
    // Start with PWM at neutral (around 1.65V)
    OCR1A = PWM_BIAS;

    printf("--- PID Control System Initialized ---\n");
    printf("SP: %u | Kp: %.2f | Ki: %.2f | Kd: %.2f\n", SETPOINT, KP, KI, KD);
    
    // Wait for everything to settle
    _delay_ms(100); 
    
    // Get an initial 'last_error' value before the loop starts
    last_error = SETPOINT - read_adc();

    for (;;) {
        // SENSE (Read Process Variable) 
        measured_value = read_adc();

        // CALCULATE ERROR
        error = SETPOINT - (int16_t)measured_value;

        // CALCULATE P, I, D TERMS
        
        // P-Term (Proportional)
        p_out = KP * error;

        // I-Term (Integral) with Anti-Windup
        integral_sum += error * LOOP_TIME_S;
        if (integral_sum > INTEGRAL_MAX) {
            integral_sum = INTEGRAL_MAX;
        } else if (integral_sum < INTEGRAL_MIN) {
            integral_sum = INTEGRAL_MIN;
        }
        i_out = KI * integral_sum;

        // D-Term (Derivative)
        double derivative = (error - last_error) / LOOP_TIME_S;
        d_out = KD * derivative;

        // CALCULATE TOTAL OUTPUT
        output = PWM_BIAS + p_out + i_out + d_out;

        // CLAMP (SATURATE) OUTPUT
        if (output > PWM_MAX) {
            output = PWM_MAX;
        } else if (output < PWM_MIN) {
            output = PWM_MIN;
        }

        // ACTUATE (Apply Output to PWM)
        OCR1A = (uint8_t)output;

        // SAVE STATE FOR NEXT LOOP
        last_error = error;

        // DEBUG & WAIT
        // Print values to serial, vital for tuning
        printf("SP:%u, PV:%u, E:%.1f, PWM:%u\n", 
               SETPOINT, measured_value, error, (uint8_t)output);
        
        _delay_ms(LOOP_TIME_MS);
    }
}