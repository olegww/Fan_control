#ifndef RPM_H
#define RPM_H

#include <Arduino.h>

void initRPM();
void monitorRPM();
extern volatile int rpm;

#endif // RPM_H
