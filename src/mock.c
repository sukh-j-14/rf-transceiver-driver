#include "mock.h"
#include <string.h>
static int read_reg(void*ctx,unsigned reg,uint32_t*out){
 mock_radio*m=ctx;if(m->fail||reg>=REG_COUNT)return -1;
 if(reg==REG_STATUS&&!m->stuck&&++m->polls>=m->delay)
  m->registers[REG_STATUS]|=m->registers[REG_CONTROL]==CTRL_TX?STATUS_DONE:STATUS_READY;
 *out=m->registers[reg];return 0;
}
static int write_reg(void*ctx,unsigned reg,uint32_t value){
 mock_radio*m=ctx;if(m->fail||reg>=REG_COUNT)return -1;
 if(reg==REG_ID||reg==REG_STATUS)return -1;
 if(reg==REG_CONTROL){
  if(value==CTRL_RESET){m->registers[REG_STATUS]=0;m->length=0;}
  else if(value==CTRL_TX)m->registers[REG_STATUS]&=~STATUS_DONE;
  else return -1;
  m->polls=0;
 }
 if(reg==REG_DATA){if(m->length==64)m->length=0;m->payload[m->length++]=(uint8_t)value;}
 m->registers[reg]=value;return 0;
}
void mock_reset(mock_radio*m){memset(m,0,sizeof(*m));m->registers[REG_ID]=0x5246;m->delay=3;}
rf_bus mock_bus(mock_radio*m){rf_bus b={m,read_reg,write_reg};return b;}
