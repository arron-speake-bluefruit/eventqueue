#include "mock_time.h"

static uint64_t time;

void mock_time_reset(void) {
    time = 0;
}

uint64_t mock_time_get(void) {
    return time;
}

void time_sleep_until(uint64_t deadline) {
    if (time < deadline) {
        time = deadline;
    }
}

uint64_t time_now_us(void) {
    return time;
}
