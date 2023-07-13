#include "eventqueue.h"

EventQueue event_queue_new(void) {
    return (EventQueue){0};
}

TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction function,
    void* userdata
) {
    (void)queue;
    (void)delay_us;
    (void)function;
    (void)userdata;

    uint32_t id = queue->next_timer_id;
    queue->next_timer_id += 1;
    return (TimerId){id};
}

void event_queue_free(EventQueue* queue) {
    (void)queue;
}
