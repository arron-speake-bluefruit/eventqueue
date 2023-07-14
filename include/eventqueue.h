#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "timer_heap.h"
#include <stddef.h>
#include <stdint.h>

typedef struct TimerId {
    uint32_t id;
} TimerId;

typedef struct EventQueue {
    uint32_t next_timer_id;
    TimerHeap timers;
} EventQueue;

EventQueue event_queue_new(void);

TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction function,
    void* userdata
);

TimerId event_queue_add_periodic_timer(
    EventQueue* queue,
    uint64_t delay_us,
    uint64_t period_us,
    TimerFunction function,
    void* userdata
);

bool event_queue_wait(EventQueue* queue);

void event_queue_free(EventQueue* queue);

#endif // EVENT_QUEUE_H
