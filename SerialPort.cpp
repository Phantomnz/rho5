#include "SerialPort.hpp"
#include <iostream>

SerialPort::SerialPort() : m_hSerial(INVALID_HANDLE_VALUE), m_connected(false) {}

SerialPort::~SerialPort() {
    disconnect();
}

bool SerialPort::connect(const std::string& portName, int baudRate) {
    // Windows requires "\\\\.\\" prefix for COM ports > 9
    std::string fullPortName = "\\\\.\\" + portName;

    m_hSerial = CreateFileA(fullPortName.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0, NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, NULL);

    if (m_hSerial == INVALID_HANDLE_VALUE) return false;

    // Set parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(m_hSerial, &dcbSerialParams)) return false;

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(m_hSerial, &dcbSerialParams)) return false;

    // Set timeouts (Non-blocking read!)
    // This is crucial for the GUI not to freeze.
    m_timeouts.ReadIntervalTimeout = MAXDWORD;
    m_timeouts.ReadTotalTimeoutConstant = 0;
    m_timeouts.ReadTotalTimeoutMultiplier = 0;
    m_timeouts.WriteTotalTimeoutConstant = 10;
    m_timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(m_hSerial, &m_timeouts)) return false;

    m_connected = true;
    return true;
}

void SerialPort::disconnect() {
    if (m_connected) {
        CloseHandle(m_hSerial);
        m_connected = false;
    }
}

bool SerialPort::isConnected() const {
    return m_connected;
}

bool SerialPort::write(const std::string& data) {
    if (!m_connected) return false;
    DWORD bytesSend;
    if (!WriteFile(m_hSerial, data.c_str(), data.size(), &bytesSend, NULL)) {
        return false;
    }
    return true;
}

int SerialPort::read(char* buffer, unsigned int bufSize) {
    if (!m_connected) return 0;
    DWORD bytesRead = 0;
    // Because we set timeouts to 0, this returns immediately
    if (ReadFile(m_hSerial, buffer, bufSize, &bytesRead, NULL)) {
        return bytesRead;
    }
    return 0;
}