#include "AVRSerial.hpp"
#include <avr/io.h> // For USART registers and bit definitions
#include <stdio.h>  // For sprintf/sscanf
#include <stdlib.h> // Required for atoi and atof
#include <avr/io.h> // Required for LED control (PORTB)

AVRSerial::AVRSerial() : m_bufferIndex(0) {
    // Configure UART1 baud rate
    #define BAUD 9600
    #include <util/setbaud.h> // Helper to calculate UBRR values
    
    // Note: We use UBRR1H/L for UART1
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
    
    // Enable RX and TX for UART1
    // Changed RXEN0/TXEN0 -> RXEN1/TXEN1
    UCSR1B = _BV(RXEN1) | _BV(TXEN1);
    
    // 8-bit data, 1 stop bit for UART1
    // Changed UCSZ01/00 -> UCSZ11/10
    UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);
}

void AVRSerial::sendData(uint16_t setpoint, uint16_t measured, uint8_t output) {
    char buffer[64];
    // Format the string into a local buffer
    snprintf(buffer, sizeof(buffer), "D,%u,%u,%u\r\n", setpoint, measured, output);
    
    // Send the buffer one char at a time
    for (int i = 0; buffer[i] != '\0'; i++) {
        // Wait for UART1 TX buffer empty (UDRE1)
        while (!(UCSR1A & _BV(UDRE1))); 
        UDR1 = buffer[i]; // Write to UDR1
    } 
}

void AVRSerial::processIncomingData(PIDController& pid, uint16_t& setpoint) {
    // 1. Check if data is available on UART1 (RXC1)
    while(UCSR1A & _BV(RXC1)) {
        char c = UDR1; // Read from UDR1

        // 2. Check for end of line
        if (c == '\n' || c == '\r') {
            m_buffer[m_bufferIndex] = '\0'; // Null-terminate
            parseCommand(pid, setpoint);    // Process the full command
            m_bufferIndex = 0;              // Reset buffer
        } else {
            // 3. Add to buffer (if safe)
            if (m_bufferIndex < sizeof(m_buffer) - 1) {
                m_buffer[m_bufferIndex++] = c;
            }
        }
    }
}

void AVRSerial::parseCommand(PIDController& pid, uint16_t& setpoint) {
    char type = m_buffer[0];
    char* value_str = &m_buffer[1];

    // Visual Debug: Toggle LED (PB7)
    PORTB ^= _BV(PB7); 

    if (type == 's') {
        setpoint = atoi(value_str); 
    } 
    else if (type == 'p') {
        double val = atof(value_str);
        pid.setGains(val, pid.getKi(), pid.getKd());
    }
    else if (type == 'i') {
        double val = atof(value_str);
        pid.setGains(pid.getKp(), val, pid.getKd());
    }
    else if (type == 'd') {
        double val = atof(value_str);
        pid.setGains(pid.getKp(), pid.getKi(), val);
    }
}