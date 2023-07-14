#ifndef EVENTQUEUE_TIME_H
#define EVENTQUEUE_TIME_H

// Time utilities. Called eq_time to not conflict with system header.

#include <stdint.h>

void time_sleep_until(uint64_t deadline);
uint64_t time_now_us(void);

#endif // EVENTQUEUE_TIME_H
