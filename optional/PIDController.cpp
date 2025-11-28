#include "config.hpp"
#include "PIDController.hpp"

PIDController::PIDController(double kp, double ki, double kd)
    : m_kp(kp), m_ki(ki), m_kd(kd), m_integral_sum(0.0), m_last_error(0.0) {} // Constructor to initialize PID gains

void PIDController::setGains(double kp, double ki, double kd) {
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
} // Method to set PID gains (what the gui will call)

void PIDController::reset() {
    m_integral_sum = 0.0;
    m_last_error = 0.0;
} // Method to reset the PID controller state

uint8_t PIDController::update(uint16_t setpoint, uint16_t measured_value) {
    double error = (double)setpoint - (double)measured_value;

    // Proportional term (proportional to current error)
    double p_out = m_kp * error;

    // Integral term (accumulated error over time) with windup guarding, no overflow
    m_integral_sum += error * LOOP_TIME_S;
    if (m_integral_sum > INTEGRAL_MAX) {
        m_integral_sum = INTEGRAL_MAX;
    } else if (m_integral_sum < INTEGRAL_MIN) {
        m_integral_sum = INTEGRAL_MIN;
    }
    double i_out = m_ki * m_integral_sum; // Integral term

    // Derivative term (based on error change rate)
    double derivative = (error - m_last_error) / LOOP_TIME_S;
    double d_out = m_kd * derivative;
    // Total output calculation
    double output_correction = p_out + i_out + d_out;

    double final_output = PWM_BIAS + output_correction;

    // Save error for next derivative calculation
    m_last_error = error;

    // Clamp output to PWM limits, no overflow
    if (final_output > PWM_MAX) {
        final_output = PWM_MAX;
    } else if (final_output < PWM_MIN) {
        final_output = PWM_MIN;
    }

    return (uint8_t)final_output;
} // Update method to compute the control output