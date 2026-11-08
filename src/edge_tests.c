#include "mock.h"
#include <stdio.h>
#include <string.h>

/* Fail exactly one selected bus operation; all others reach the mock. */
typedef struct {
 rf_bus inner;
 unsigned calls, fail_at;
} fault_bus;
static int fault_read(void *ctx,unsigned reg,uint32_t *value){
 fault_bus *f=ctx;
 if(++f->calls==f->fail_at)return -1;
 return f->inner.read(f->inner.context,reg,value);
}
static int fault_write(void *ctx,unsigned reg,uint32_t value){
 fault_bus *f=ctx;
 if(++f->calls==f->fail_at)return -1;
 return f->inner.write(f->inner.context,reg,value);
}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}checks++;}while(0)
int main(void){
 unsigned checks=0;
 mock_radio m;
 radio r={0};
 uint8_t payload[64];
 memset(payload,0xa5,sizeof(payload));
 mock_reset(&m);
 fault_bus f={mock_bus(&m),0,0};
 rf_bus b={&f,fault_read,fault_write};
 rf_bus invalid=b;
 invalid.read=NULL;
 CHECK(radio_init(&r,invalid,3)==RF_INVALID);
 invalid=b;invalid.write=NULL;
 CHECK(radio_init(&r,invalid,3)==RF_INVALID);
 CHECK(radio_init(&r,b,0)==RF_INVALID);
 CHECK(f.calls==0);
 CHECK(radio_tune(NULL,700000)==RF_STATE);
 CHECK(radio_send(NULL,payload,1,3)==RF_STATE);
 /* ID read, reset write, then each of the three readiness reads. */
 for(unsigned fail=1;fail<=5;fail++){
  mock_reset(&m);f.calls=0;f.fail_at=fail;
  CHECK(radio_init(&r,b,3)==RF_IO);
  CHECK(!r.initialized&&r.frequency_khz==0);
  CHECK(f.calls==fail);
 }
 f.fail_at=0;f.calls=0;
 CHECK(radio_init(&r,b,3)==RF_OK);
 CHECK(m.polls==3);
 CHECK(radio_tune(&r,700000)==RF_OK);
 f.calls=0;
 CHECK(radio_send(&r,payload,0,3)==RF_INVALID);
 CHECK(radio_send(&r,payload,1,0)==RF_INVALID);
 CHECK(f.calls==0);
 CHECK(radio_send(&r,payload,sizeof(payload),3)==RF_OK);
 CHECK(m.length==64&&memcmp(m.payload,payload,64)==0);
 /* A previous DONE must not satisfy a new transaction's short budget. */
 CHECK(radio_send(&r,payload,1,2)==RF_TIMEOUT);
 CHECK(m.polls==2);
 /* Four data writes, TX write, then each completion read can fail. */
 for(unsigned fail=1;fail<=8;fail++){
  mock_reset(&m);f.calls=0;f.fail_at=0;
  CHECK(radio_init(&r,b,3)==RF_OK);
  CHECK(radio_tune(&r,900000)==RF_OK);
  f.calls=0;f.fail_at=fail;
  CHECK(radio_send(&r,payload,4,3)==RF_IO);
  CHECK(f.calls==fail);
  /* Reset explicitly discards any partially staged payload. */
  f.fail_at=0;
  CHECK(radio_init(&r,b,3)==RF_OK);
  CHECK(m.length==0&&r.frequency_khz==0);
  CHECK(radio_send(&r,payload,4,3)==RF_STATE);
  CHECK(radio_tune(&r,900000)==RF_OK);
  CHECK(radio_send(&r,payload,4,3)==RF_OK);
  CHECK(m.length==4&&memcmp(m.payload,payload,4)==0);
 }
 CHECK(strcmp(radio_error(RF_OK),"OK")==0);
 CHECK(strcmp(radio_error(RF_STATE),"STATE")==0);
 CHECK(strcmp(radio_error((rf_result)-1),"UNKNOWN")==0);
 CHECK(strcmp(radio_error((rf_result)99),"UNKNOWN")==0);
 printf("PASS %u edge checks: operation faults, poll budgets, reset recovery\n",checks);
 return 0;
}
