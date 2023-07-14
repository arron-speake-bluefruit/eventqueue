#ifndef EVENTQUEUE_TIMER_HEAP_H
#define EVENTQUEUE_TIMER_HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*TimerFunction)(void* userdata);

typedef struct TimerId {
    uint32_t id;
} TimerId;

typedef struct Timer {
    // When the timer should be fired next.
    uint64_t deadline;

    // The period of the timer. If equal to `UINT64_MAX`, is a one-shot timer.
    uint64_t period;

    // The function to call when the timer fires.
    TimerFunction function;

    // Data to pass to `function when its called.
    void* userdata;

    // The ID of the timer.
    uint32_t id;
} Timer;

typedef struct TimerHeap {
    Timer* data;
    size_t size;
    size_t capacity;
} TimerHeap;

TimerHeap timer_heap_new(void);
void timer_heap_insert(TimerHeap* heap, Timer timer);
const Timer* timer_heap_find(const TimerHeap* heap);
bool timer_heap_take(TimerHeap* heap, Timer* out);
void timer_heap_remove_id(TimerHeap* heap, TimerId id);
void timer_heap_free(TimerHeap* heap);

#endif // EVENTQUEUE_TIMER_HEAP_H
