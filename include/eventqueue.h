#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdint.h>

typedef void (*TimerFunction)(void* userdata);

typedef struct TimerId {
    uint32_t id;
} TimerId;

typedef struct EventQueue {
    uint32_t next_timer_id;
} EventQueue;

EventQueue event_queue_new(void);

TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction function,
    void* userdata
);

void event_queue_free(EventQueue* queue);

#endif // EVENT_QUEUE_H
