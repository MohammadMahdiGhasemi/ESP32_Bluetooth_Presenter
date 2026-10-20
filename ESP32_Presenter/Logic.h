#pragma once
#include <stdint.h>
struct NumberInput {
 unsigned value=0, count=0; uint32_t last=0;
 void clear(){value=0;count=0;}
 void digit(unsigned d,uint32_t now){
  if(d>9 || count>=4)return;
  value=value*10+d; ++count;last=now;
 }
 bool ready(uint32_t now)const{return count && value>0 && uint32_t(now-last)>=1000;}
};
