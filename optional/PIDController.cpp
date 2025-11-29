#include "config.hpp"
#include "PIDController.hpp"

PIDController::PIDController(double kp, double ki, double kd)
    : m_kp(kp), m_ki(ki), m_kd(kd), m_integral_sum(0.0), m_last_error(0.0) {}

void PIDController::setGains(double kp, double ki, double kd) {
    m_kp = kp; 
    m_ki = ki; 
    m_kd = kd;
    // [CRITICAL FIX] Wipe memory when gains change to prevent "Shock"
    reset(); 
} 

void PIDController::reset() {
    m_integral_sum = 0.0; 
    m_last_error = 0.0;
} 

uint8_t PIDController::update(uint16_t setpoint, uint16_t measured_value) {
    // 1. Safety Reset on Zero
    if (setpoint == 0) {
        reset();
        return 0;
    }

    double error = (double)setpoint - (double)measured_value;
    double feedforward = (double)setpoint / 4.0;

    // 2. Proportional
    double p_out = m_kp * error;

    // 3. Integral with Tighter Clamping (Speed up recovery)
    m_integral_sum += error * LOOP_TIME_S;
    if (m_integral_sum > 500.0) m_integral_sum = 500.0;
    else if (m_integral_sum < -500.0) m_integral_sum = -500.0;
    
    double i_out = m_ki * m_integral_sum; 

    // 4. Derivative
    double derivative = (error - m_last_error) / LOOP_TIME_S;
    double d_out = m_kd * derivative;

    // 5. Final Output
    double final_output = feedforward + p_out + i_out + d_out;
    m_last_error = error;

    if (final_output > 255.0) return 255;
    if (final_output < 0.0) return 0;

    return (uint8_t)final_output;
}