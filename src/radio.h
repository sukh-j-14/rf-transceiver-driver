#ifndef RADIO_H
#define RADIO_H
#include <stdint.h>
#include <stddef.h>
typedef enum { RF_OK, RF_INVALID, RF_IO, RF_TIMEOUT, RF_STATE } rf_result;
enum { REG_ID, REG_CONTROL, REG_STATUS, REG_FREQ, REG_DATA, REG_COUNT };
enum { CTRL_RESET=1, CTRL_TX=2, STATUS_READY=1, STATUS_DONE=2 };
typedef struct { void *context; int (*read)(void*,unsigned,uint32_t*); int (*write)(void*,unsigned,uint32_t); } rf_bus;
typedef struct { rf_bus bus; int initialized; uint32_t frequency_khz; } radio;
rf_result radio_init(radio*,rf_bus,unsigned);
rf_result radio_tune(radio*,uint32_t);
rf_result radio_send(radio*,const uint8_t*,size_t,unsigned);
const char *radio_error(rf_result);
#endif
