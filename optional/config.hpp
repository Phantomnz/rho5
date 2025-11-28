#pragma once

// INTIAL GAINS AND SETPOINT
#define INITIAL_SETPOINT 512 // Midpoint for a 10-bit ADC (0-1023)
#define INITIAL_KP 0.5 // Proportional gain
#define INITIAL_KI 0.1 // Integral gain
#define INITIAL_KD 0.05 // Derivative gain

// SYSTEM TIMING
#define LOOP_TIME_MS 10 // Loop time in milliseconds
#define LOOP_TIME_S (LOOP_TIME_MS / 1000.0) // Convert ms to seconds

// HARDWARE AND LOGIC LIMITS
#define PWM_MIN 0 // Minimum PWM value
#define PWM_MAX 255 // Assuming an 8-bit PWM resolution
#define PWM_BIAS 128 // Neutral PWM value for bidirectional control
#define INTEGRAL_MAX 50.0 // To prevent integral windup
#define INTEGRAL_MIN -50.0 // To prevent integral windup

