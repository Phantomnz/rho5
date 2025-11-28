#pragma once
#include <stdint.h>

class ADCReader {
public:
    ADCReader();
    uint16_t readADC(uint8_t channel);
};