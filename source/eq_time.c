#define _POSIX_C_SOURCE 200809L // For POSIX clock_* functions
#include "eq_time.h"
#include <assert.h>
#include <time.h>

static struct timespec microseconds_to_timespec(uint64_t microseconds) {
    return (struct timespec){
        .tv_sec = microseconds / 1000000UL,
        .tv_nsec = (microseconds % 1000000UL) * 1000UL,
    };
}

static uint64_t timespec_to_microseconds(struct timespec timespec) {
    return ((uint64_t)timespec.tv_sec * 1000000UL)
        + ((uint64_t)timespec.tv_nsec / 1000UL);
}

void time_sleep_until(uint64_t deadline) {
    struct timespec timespec = microseconds_to_timespec(deadline);
    int status = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timespec, NULL);
    assert(status == 0);
    // TODO: When signal handling is integrated, check the status here for interrupted sleep.
}

uint64_t time_now_us(void) {
    struct timespec timespec;
    clock_gettime(CLOCK_MONOTONIC, &timespec);
    return timespec_to_microseconds(timespec);
}
