#include "radio.h"
static rf_result wait_for(radio *r,uint32_t mask,unsigned polls){
 uint32_t s; for(unsigned i=0;i<polls;i++){
  if(r->bus.read(r->bus.context,REG_STATUS,&s))return RF_IO;
  if((s&mask)==mask)return RF_OK;
 }return RF_TIMEOUT;
}
rf_result radio_init(radio*r,rf_bus b,unsigned polls){
 uint32_t id;if(!r||!b.read||!b.write||!polls)return RF_INVALID;
 r->bus=b;r->initialized=0;r->frequency_khz=0;
 if(b.read(b.context,REG_ID,&id))return RF_IO;
 if(id!=0x5246)return RF_INVALID;
 if(b.write(b.context,REG_CONTROL,CTRL_RESET))return RF_IO;
 rf_result result=wait_for(r,STATUS_READY,polls);
 if(result==RF_OK)r->initialized=1;
 return result;
}
rf_result radio_tune(radio*r,uint32_t khz){
 if(!r||!r->initialized)return RF_STATE;
 if(khz<700000||khz>6000000)return RF_INVALID;
 if(r->bus.write(r->bus.context,REG_FREQ,khz))return RF_IO;
 r->frequency_khz=khz;return RF_OK;
}
rf_result radio_send(radio*r,const uint8_t*data,size_t n,unsigned polls){
 if(!r||!r->initialized||!r->frequency_khz)return RF_STATE;
 if(!data||n==0||n>64||!polls)return RF_INVALID;
 for(size_t i=0;i<n;i++)if(r->bus.write(r->bus.context,REG_DATA,data[i]))return RF_IO;
 if(r->bus.write(r->bus.context,REG_CONTROL,CTRL_TX))return RF_IO;
 return wait_for(r,STATUS_DONE,polls);
}
const char *radio_error(rf_result e){
 static const char *names[]={"OK","INVALID","IO","TIMEOUT","STATE"};
 return (unsigned)e<5?names[e]:"UNKNOWN";
}
