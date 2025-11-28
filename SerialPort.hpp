#pragma once
#include <windows.h>
#include <string>
#include <vector>

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    // Connect to a port (e.g., "COM3")
    bool connect(const std::string& portName, int baudRate = 9600);
    
    // Disconnect
    void disconnect();
    
    // Check if connected
    bool isConnected() const;

    // Send data to the Il Matto
    // We'll send strings like "s512\n"
    bool write(const std::string& data);

    // Read all available data from the Il Matto
    // Returns size of data read
    int read(char* buffer, unsigned int bufSize);

private:
    HANDLE m_hSerial; // Windows handle to the port
    bool m_connected;
    COMMTIMEOUTS m_timeouts;
};