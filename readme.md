# eventqueue generic event queue library

Features:
- Periodic events (triggers every X microseconds)
- Internal events (triggers on calls to a event push function)

Maybe features:
- I/O events (triggers on pipes/files/sockets/etc.)
- Priority/Urgency (priority level integer OR urgent flag)
- Thread-safety (Just firing internal events OR entire stack)
- Interrupt safety/re-entrancy on event-trigger function calls?
- Catch signals?
- Specify event delays

# Periodic events

Able to register (and deregister) periodic events which fire at a fixed interval.

```c
typedef void (*TimerFunction)(void* userdata);

typedef struct TimerId { uint32_t id; } TimerId;

// Add a one-shot timer event, which calls `function(userdata)` exactly once after `delay_us`
// microseconds.
TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction function,
    void* userdata
);

// Add a repeating timer event, which (after `delay_us` microseconds passes) calls
// `function(userdata)` every `period_us` microseconds.
TimerId event_queue_add_periodic_timer(
    EventQueue* queue,
    uint64_t delay_us,
    uint64_t period_us,
    TimerFunction function,
    void* userdata
);

// Cancel and remove a timer event. After calling this, the associated function will not be called.
void event_queue_remove_timer(EventQueue* queue, TimerId id);
```

# I/O events

Able to register (and deregister) event triggers associated with file I/O. Events for when a fd:
- has data to be read without blocking
- can be written to without blocking
- an error occurs on the device/stream
- a device is disconnected/a pipe is closed/etc.

```c

typedef void (*IoEventFunction)(int fd, void* userdata);

// A kind of I/O event to watch for.
typedef enum IoEventKind {
    // Data is available to read on a device/stream without blocking.
    io_event_kind_read,

    // It is possible to write data to a device/stream without blocking.
    io_event_kind_write,

    // An error occured in the device/stream.
    io_event_kind_error,

    // The device/stream closes or disconnected.
    io_event_kind_hangup,
} IoEventKind;

// Registers `function(fd, userdata)` to be called when the event associated with `kind` occurs.
IoEventId event_queue_add_io_event(
    EventQueue* queue,
    int fd,
    IoEventKind kind,
    IoEventFunction function,
    void* userdata
);

// Deregister the event identified by `id`. After this call, the associated function will not be
// called.
void event_queue_remove_io_event(EventQueue* queue, IoEventId id);
```

# Internal events

Able to register, deregister and trigger events from other parts of a program.

```c
typedef struct EventId { uint32_t id; } EventId;

typedef void (*EventFunction)(void* userdata, void* eventdata);

// Add an event which can be called by passing the returned event id around. `userdata` is passed
// to all invocations of the event.
EventId event_queue_add_event(EventQueue* queue, EventFunction function, void* userdata);

// Unregister the event with the given id.
void event_queue_remove_event(EventQueue* queue, EventId id);

// Trigger the event given by `id`. Results in `function(userdata, eventdata)` being called once.
// See `event_queue_add_event`.
void event_queue_trigger_event(EventQueue* queue, EventId id, void* eventdata);
```
