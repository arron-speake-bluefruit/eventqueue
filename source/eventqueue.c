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
        .period = UINT64_MAX,
        .function = function,
        .userdata = userdata,
        .id = id,
    };

    timer_heap_insert(&queue->timers, timer);

    return (TimerId){id};
}

TimerId event_queue_add_periodic_timer(
    EventQueue* queue,
    uint64_t delay_us,
    uint64_t period_us,
    TimerFunction function,
    void* userdata
) {
    uint32_t id = queue->next_timer_id;
    queue->next_timer_id += 1;

    uint64_t now = time_now_us();

    Timer timer = {
        .deadline = now + delay_us,
        .period = period_us,
        .function = function,
        .userdata = userdata,
        .id = id,
    };

    timer_heap_insert(&queue->timers, timer);

    return (TimerId){id};
}

void event_queue_remove_timer(EventQueue* queue, TimerId id) {
    timer_heap_remove_id(&queue->timers, id);
}

bool event_queue_wait(EventQueue* queue) {
    Timer timer;
    if (timer_heap_take(&queue->timers, &timer)) {
        time_sleep_until(timer.deadline);
        timer.function(timer.userdata);

        bool is_periodic = timer.period != UINT64_MAX;
        if (is_periodic) {
            // TODO: This can be optimized by checking if the timer is reinserted before the binary
            // tree sift-down in timer_heap_take.
            timer.deadline += timer.period;
            timer_heap_insert(&queue->timers, timer);
        }

        return true;
    } else {
        return false;
    }
}

void event_queue_free(EventQueue* queue) {
    timer_heap_free(&queue->timers);
}
