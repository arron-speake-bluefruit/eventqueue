#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "timer_heap.h"
#include <stddef.h>
#include <stdint.h>

// A kind of I/O event to watch for.
typedef enum EventIoFlag {
    // Data is available to read on a device/stream without blocking.
    event_io_flag_read = (1 << 0),

    // It is possible to write data to a device/stream without blocking. Unimplemented.
    // event_io_flag_write = (1 << 1),

    // An error occured in the device/stream. Unimplemented.
    // event_io_flag_error = (1 << 2),

    // The device/stream closes or disconnected. Unimplemented.
    // event_io_flag_hangup = (1 << 3),
} EventIoFlag;

// A function called by an I/O event. `userdata` is the value given to `event_queue_add_io_event`,
// `fd` is the corresponding file descriptor, and `flag` is the kind of event which triggered this call.
typedef void (*EventIoFunction)(int fd, EventIoFlag flag, void* userdata);

// A function called by an internal event. `userdata` is provided when registering the event,
// `eventdata` is passed when triggering the event.
typedef void (*EventFunction)(void* userdata, void* eventdata);

// The ID of a registered event. Used to remove and trigger events.
typedef struct EventId {
    uint32_t id;
} EventId;

// The ID of a registered I/O event. Used to remove I/O events.
typedef struct IoEventId {
    uint32_t id;
} IoEventId;

// Internal event information
typedef struct Event Event;

// An event queue.
typedef struct EventQueue {
    uint32_t next_timer_id;
    uint32_t next_event_id;
    TimerHeap timers;
    Event* events;
    size_t events_size;
    size_t events_capacity;

    bool has_io_event;
    int io_fd;
    uint32_t io_mask;
    EventIoFunction io_function;
    void* io_userdata;
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

// Given a `mask` (one or more EventIoFlag values OR'd together) and a file descriptor (`fd`),
// trigger a call to `function(fd, flag, userdata)` when a corresponding I/O event occurs.
IoEventId event_queue_add_io_event(
    EventQueue* queue,
    int fd,
    uint32_t mask,
    EventIoFunction function,
    void* userdata
);

// Remove an I/O event (identified by `id`) from the event queue. Associated functions
void event_queue_remove_io_event(EventQueue* queue, IoEventId id);

// If there are no events to wait for, return false immediately. Otherwise, wait until the next
// event can be processed, process it, and return true.
bool event_queue_wait(EventQueue* queue);

// Free all resources owned by the event queue. No timers or events will be called, and all IDs
// become invalid.
void event_queue_free(EventQueue* queue);

#endif // EVENT_QUEUE_H
