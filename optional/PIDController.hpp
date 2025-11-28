#pragma once
#include <stdint.h>

class PIDController {
    public:
    PIDController(double kp, double ki, double kd); // Constructor to initialize PID gains

    uint8_t update(uint16_t setpoint, uint16_t measured_value); // Update method to compute the control output

    void setGains(double kp, double ki, double kd); // Method to set PID gains (what the gui will call)

    void reset(); // Method to reset the PID controller state

    double getKp() const { return m_kp; }
    double getKi() const { return m_ki; }
    double getKd() const { return m_kd; }

    private:
    double m_kp; // Proportional gain
    double m_ki; // Integral gain
    double m_kd; // Derivative gain

    double m_integral_sum; // Integral term, accumulated over time
    double m_last_error; // Previous error for derivative calculation
};