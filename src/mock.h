#ifndef MOCK_H
#define MOCK_H
#include "radio.h"
typedef struct { uint32_t registers[REG_COUNT]; unsigned polls,delay; int fail,stuck; uint8_t payload[64]; size_t length; } mock_radio;
void mock_reset(mock_radio*);
rf_bus mock_bus(mock_radio*);
#endif
