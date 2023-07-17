#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "timer_heap.h"
#include <stddef.h>
#include <stdint.h>

typedef void (*EventFunction)(void* userdata, void* eventdata);

typedef struct EventId {
    uint32_t id;
} EventId;

typedef struct Event {
    uint32_t id;
    void* userdata;
    EventFunction function;
} Event;

typedef struct EventQueue {
    uint32_t next_timer_id;
    uint32_t next_event_id;
    TimerHeap timers;

    Event* events;
    size_t events_size;
    size_t events_capacity;
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

void event_queue_remove_timer(EventQueue* queue, TimerId id);

EventId event_queue_add_event(EventQueue* queue, EventFunction function, void* userdata);

void event_queue_remove_event(EventQueue* queue, EventId id);

void event_queue_trigger_event(EventQueue* queue, EventId id, void* eventdata);

bool event_queue_wait(EventQueue* queue);

void event_queue_free(EventQueue* queue);

#endif // EVENT_QUEUE_H
