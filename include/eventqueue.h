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

// Create a new event queue with no registered timers or events.
EventQueue event_queue_new(void);

// Add a one-shot timer to the event queue. `function(userdata)` will be called after `delay_us`
// time has passed.
TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction function,
    void* userdata
);

// Add a repeating timer to the event queue. After an initial delay of `delay_us`,
// `function(userdata)` will be called every `period_us`.
TimerId event_queue_add_periodic_timer(
    EventQueue* queue,
    uint64_t delay_us,
    uint64_t period_us,
    TimerFunction function,
    void* userdata
);

// Remove a timer (identified by `id`) from the event queue. The assocaited function will not be
// called afterwards.
void event_queue_remove_timer(EventQueue* queue, TimerId id);

// Register an event with the event queue. See `event_queue_trigger_event`.
EventId event_queue_add_event(EventQueue* queue, EventFunction function, void* userdata);

// Remove an event from the event queue. Unprocessed triggered events of this ID will not be
// called. Future triggers for this ID will be ignored.
void event_queue_remove_event(EventQueue* queue, EventId id);

// Trigger an event with the given `id`. Will result in a call of `function(userdata, eventdata)`
// given the event's function and userdata. (See `event_queue_add_event`).
void event_queue_trigger_event(EventQueue* queue, EventId id, void* eventdata);

// If there are no events to wait for, return false immediately. Otherwise, wait until the next
// event can be processed, process it, and return true.
bool event_queue_wait(EventQueue* queue);

// Free all resources owned by the event queue. No timers or events will be called, and all IDs
// become invalid.
void event_queue_free(EventQueue* queue);

#endif // EVENT_QUEUE_H
