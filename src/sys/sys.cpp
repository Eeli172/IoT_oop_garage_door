#include "sys.h"
#include <any>
#include <iostream>


Sys::Sys() :
sysState(PRE_CALIB), calibrationStatus(false), encoderPos(0), doorState(CLOSED)
{}
