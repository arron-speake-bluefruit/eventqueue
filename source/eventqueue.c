#include "eventqueue.h"
#include "eq_time.h"
#include <stdlib.h>
#include <string.h>

static void reallocate_events_if_at_capacity(EventQueue* queue) {
    if (queue->events_size == queue->events_capacity) {
        queue->events_capacity *= 2;
        queue->events = realloc(queue->events, sizeof(Event) * queue->events_capacity);
        if (queue->events == NULL) abort();
    }
}

static void push_event(EventQueue* queue, Event event) {
    reallocate_events_if_at_capacity(queue);
    queue->events[queue->events_size] = event;
    queue->events_size += 1;
}

EventQueue event_queue_new(void) {
    TimerHeap timers = timer_heap_new();

    Event* events = malloc(sizeof(Event));
    if (events == NULL) abort();

    return (EventQueue){
        .next_timer_id = 0,
        .next_event_id = 0,
        .timers = timers,
        .events = events,
        .events_size = 0,
        .events_capacity = 1,
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

EventId event_queue_add_event(EventQueue* queue, EventFunction function, void* userdata) {
    uint32_t id = queue->next_event_id;
    queue->next_event_id += 1;

    Event event = {
        .id = id,
        .function = function,
        .userdata = userdata,
    };

    push_event(queue, event);

    return (EventId){id};
}

static bool get_event_by_id(const EventQueue* queue, EventId id, size_t* out) {
    // TODO: Replace with binary search
    for (size_t i = 0; i < queue->events_size; i++) {
        if (queue->events[i].id == id.id) {
            *out = i;
            return true;
        }
    }

    return false;
}

static void remove_event_at_position(EventQueue* queue, size_t index) {
    void* destination =  &queue->events[index];
    const void* source = &queue->events[index + 1];
    size_t size = sizeof(Event) * (queue->events_size - index - 1);
    memmove(destination, source, size);

    queue->events_size -= 1;
}

void event_queue_remove_event(EventQueue* queue, EventId id) {
    size_t index;
    if (get_event_by_id(queue, id, &index)) {
        remove_event_at_position(queue, index);
    }
    // TODO: Handle case of invalid ID?
}

void event_queue_trigger_event(EventQueue* queue, EventId id, void* eventdata) {
    size_t index;
    if (get_event_by_id(queue, id, &index)) {
        Event event = queue->events[index];
        event.function(event.userdata, eventdata);
    }
    // TODO: Handle case of invalid ID?
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
    free(queue->events);
}
