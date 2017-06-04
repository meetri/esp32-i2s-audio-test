#include "Minmax.h"
#include "stdio.h"
#include "stdint.h"

uint16_t Minmax::getPeakToPeak(){
    return maxsample-minsample;
}

uint16_t Minmax::getMin(){
    return minsample;
}

uint16_t Minmax::getMax(){
    return maxsample;
}

float Minmax::getVolts(){
    float v = getPeakToPeak() * 5.0 / 65535;
    //double v = this->getPeakToPeak();
    reset();
    return v;
}

void Minmax::reset(){
    maxsample = 0;
    minsample = 65535;
}

void Minmax::set( uint16_t val ){
    if ( val > maxsample ){
        maxsample = val;
    }else if ( val < minsample ){
        minsample = val;
    }
}
