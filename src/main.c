#include "mock.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}checks++;}while(0)
int main(void){
 unsigned checks=0;mock_radio m;radio r;mock_reset(&m);rf_bus b=mock_bus(&m);uint8_t data[]={1,2,3,255};
 CHECK(radio_init(NULL,b,10)==RF_INVALID);
 CHECK(radio_init(&r,b,2)==RF_TIMEOUT);
 CHECK(radio_tune(&r,900000)==RF_STATE);
 CHECK(radio_init(&r,b,3)==RF_OK);
 CHECK(radio_send(&r,data,4,10)==RF_STATE);
 CHECK(radio_tune(&r,699999)==RF_INVALID);
 CHECK(radio_tune(&r,6000001)==RF_INVALID);
 CHECK(radio_tune(&r,700000)==RF_OK);
 CHECK(radio_tune(&r,6000000)==RF_OK);
 CHECK(radio_tune(&r,3500000)==RF_OK);
 CHECK(radio_send(&r,NULL,4,10)==RF_INVALID);
 CHECK(radio_send(&r,data,65,10)==RF_INVALID);
 CHECK(radio_send(&r,data,4,3)==RF_OK);
 CHECK(m.length==4&&memcmp(m.payload,data,4)==0);
 CHECK(radio_send(&r,data,4,2)==RF_TIMEOUT);
 CHECK(radio_send(&r,data,4,3)==RF_OK); /* stale DONE must not hide timeout */
 m.fail=1;CHECK(radio_send(&r,data,4,3)==RF_IO);
 CHECK(radio_tune(&r,900000)==RF_IO);CHECK(r.frequency_khz==3500000);
 CHECK(radio_init(&r,b,10)==RF_IO);CHECK(!r.initialized);
 m.fail=0;m.stuck=1;CHECK(radio_init(&r,b,10)==RF_TIMEOUT);
 m.stuck=0;CHECK(radio_init(&r,b,10)==RF_OK);
 m.registers[REG_ID]=0;CHECK(radio_init(&r,b,10)==RF_INVALID);
 printf("PASS %u driver checks: boundaries, bus faults, timeout, recovery, payload\n",checks);
 return 0;
}
