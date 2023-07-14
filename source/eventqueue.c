#include "eventqueue.h"
#include "eq_time.h"
#include <stdlib.h>

EventQueue event_queue_new(void) {
    TimerHeap timers = timer_heap_new();

    return (EventQueue){
        .next_timer_id = 0,
        .timers = timers,
    };
}

TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction function,
    void* userdata
) {
    uint32_t id = queue->next_timer_id;
    queue->next_timer_id += 1;

    uint64_t now = time_now_us();

    Timer timer = {
        .deadline = now + delay_us,
        .function = function,
        .userdata = userdata,
        .id = id,
    };

    timer_heap_insert(&queue->timers, timer);

    return (TimerId){id};
}

void event_queue_wait(EventQueue* queue) {
    Timer timer;
    if (timer_heap_take(&queue->timers, &timer)) {
        time_sleep_until(timer.deadline);
        timer.function(timer.userdata);
    }
}

void event_queue_free(EventQueue* queue) {
    timer_heap_free(&queue->timers);
}
