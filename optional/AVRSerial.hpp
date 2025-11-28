#pragma once
#include <stdint.h>
#include "PIDController.hpp"

class AVRSerial {
public:
    AVRSerial(); // Constructor to initialize serial communication
    void sendData(uint16_t setpoint, uint16_t measured, uint8_t output); // Method to send data over serial
    void processIncomingData(PIDController& pid, uint16_t& setpoint); // Method to process incoming data and update PID gains or setpoint
private:
    char m_buffer[32]; // Buffer to hold incoming data
    uint8_t m_bufferIndex; // Current index in the buffer
    void parseCommand(PIDController& pid, uint16_t& setpoint); // Method to parse and execute commands
};