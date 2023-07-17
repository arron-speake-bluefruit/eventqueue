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
    // Whether this timer is for an event firing, and not a timer.
    bool is_event;

    // The ID of the timer. For events, the ID of the event.
    uint32_t id;

    // When the timer should be fired next. For events, its the scheduled deadline of the event.
    uint64_t deadline;

    // The period of the timer. If equal to `UINT64_MAX`, is a one-shot timer. Ignored for events.
    uint64_t period;

    // The function to call when the timer fires. Ignored for events.
    TimerFunction function;

    // Data to pass to the associated timer/event function when called.
    void* userdata;
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
