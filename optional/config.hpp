#pragma once
#define INITIAL_SETPOINT 0 
#define INITIAL_KP 0.5 
#define INITIAL_KI 0.1 
#define INITIAL_KD 0.00 

#define LOOP_TIME_MS 10 
#define LOOP_TIME_S (LOOP_TIME_MS / 1000.0)

#define PWM_MIN 0 
#define PWM_MAX 255 
#define PWM_BIAS 0 
#define INTEGRAL_MAX 1000.0 
#define INTEGRAL_MIN -1000.0