#pragma once

#include <stdint.h>
#include <stdio.h>

class Minmax {
 
public:
    void set( uint16_t val );   
    uint16_t getMin();   
    uint16_t getMax();   
    float getVolts();   
    uint16_t getPeakToPeak();
    void reset();

private:
  uint32_t maxsample;
  uint32_t minsample;

};
