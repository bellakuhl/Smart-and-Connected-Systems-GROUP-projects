#ifndef __T15_PID_CONTROL__
#define __T15_PID_CONTROL__

void PID_init();
float PID(float measured_value);
void PID_set_setpoint(float sp);

#endif

