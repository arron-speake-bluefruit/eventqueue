#include "eventqueue.h"
#include "eq_time.h"
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <assert.h>

// Definition of typedef struct Event Event (in header);
struct Event {
    uint32_t id;
    void* userdata;
    EventFunction callback;
};

// Definition of typedef struct IoEvent IoEvent (in header):
struct IoEvent {
    uint32_t id;
    EventIoFunction callback;
    void* userdata;
};

static void reallocate_events_if_at_capacity(EventQueue* queue) {
    if (queue->events_size == queue->events_capacity) {
        queue->events_capacity *= 2;
        queue->events = realloc(queue->events, sizeof(Event) * queue->events_capacity);
        if (queue->events == NULL) abort();
    }
}

void reallocate_io_events_if_at_capacity(EventQueue* queue) {
    if (queue->io_events_size == queue->io_events_capacity) {
        queue->io_events_capacity *= 2;

        queue->io_events = realloc(queue->io_events, sizeof(IoEvent) * queue->io_events_capacity);
        if (queue->io_events == NULL) abort();

        queue->io_poll_descriptors = realloc(
            queue->io_poll_descriptors, sizeof(struct pollfd) * queue->io_events_capacity);
        if (queue->io_poll_descriptors == NULL) abort();
    }
}

static void push_event(EventQueue* queue, Event event) {
    reallocate_events_if_at_capacity(queue);
    queue->events[queue->events_size] = event;
    queue->events_size += 1;
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

static bool get_io_event_by_id(const EventQueue* queue, IoEventId id, size_t* out) {
    // TODO: Replace with binary search
    for (size_t i = 0; i < queue->io_events_size; i++) {
        if (queue->io_events[i].id == id.id) {
            *out = i;
            return true;
        }
    }

    return false;
}

// given `array`, containing `length` elements of size `size`, remove the `index`th element, and
// shift subsequent element down by 1.
static void remove_array_element(size_t size, size_t length, void* array, size_t index) {
    void* destination =  (char*)array + (size * index);
    const void* source = (char*)array + (size * (index + 1));
    size_t amount = size * (length - index - 1);
    memmove(destination, source, amount);
}

static void remove_event_at_position(EventQueue* queue, size_t index) {
    remove_array_element(sizeof(Event), queue->events_size, queue->events, index);
    queue->events_size -= 1;
}

static void remove_io_event_at_position(EventQueue* queue, size_t index) {
    remove_array_element(sizeof(IoEvent), queue->io_events_size, queue->io_events, index);
    remove_array_element(sizeof(struct pollfd), queue->io_events_size, queue->io_poll_descriptors, index);
    queue->io_events_size -= 1;
}

static void push_event_to_timer_queue(EventQueue* queue, EventId id, void* eventdata) {
    Timer timer = {
        .is_event = true,
        .deadline = time_now_us(),
        .period = 0, // Unused
        .callback = NULL, // Unused
        .userdata = eventdata,
        .id = id.id,
    };
    timer_heap_insert(&queue->timers, timer);
}

EventQueue event_queue_new(void) {
    TimerHeap timers = timer_heap_new();

    Event* events = malloc(sizeof(Event));
    if (events == NULL) abort();

    struct pollfd* io_poll_descriptors = malloc(sizeof(struct pollfd));
    if (io_poll_descriptors == NULL) abort();

    IoEvent* io_events = malloc(sizeof(IoEvent));
    if (io_events == NULL) abort();

    return (EventQueue){
        .next_timer_id = 0,
        .next_event_id = 0,
        .timers = timers,
        .events = events,
        .events_size = 0,
        .events_capacity = 1,
        .io_poll_descriptors = io_poll_descriptors,
        .io_events = io_events,
        .io_events_size = 0,
        .io_events_capacity = 1,
    };
}

TimerId event_queue_add_timer(
    EventQueue* queue,
    uint64_t delay_us,
    TimerFunction callback,
    void* userdata
) {
    return event_queue_add_periodic_timer(queue, delay_us, TIMER_APERIODIC, callback, userdata);
}

TimerId event_queue_add_periodic_timer(
    EventQueue* queue,
    uint64_t delay_us,
    uint64_t period_us,
    TimerFunction callback,
    void* userdata
) {
    uint32_t id = queue->next_timer_id;
    queue->next_timer_id += 1;

    uint64_t now = time_now_us();

    Timer timer = {
        .is_event = false,
        .id = id,
        .deadline = now + delay_us,
        .period = period_us,
        .callback = callback,
        .userdata = userdata,
    };

    timer_heap_insert(&queue->timers, timer);

    return (TimerId){id};
}

void event_queue_remove_timer(EventQueue* queue, TimerId id) {
    timer_heap_remove_id(&queue->timers, id);
}

EventId event_queue_add_event(EventQueue* queue, EventFunction callback, void* userdata) {
    uint32_t id = queue->next_event_id;
    queue->next_event_id += 1;

    Event event = {
        .id = id,
        .callback = callback,
        .userdata = userdata,
    };

    push_event(queue, event);

    return (EventId){id};
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
        push_event_to_timer_queue(queue, id, eventdata);
    }
    // TODO: Handle case of invalid ID?
}

IoEventId event_queue_add_io_event(
    EventQueue* queue,
    int fd,
    uint32_t mask,
    EventIoFunction callback,
    void* userdata
) {
    // TODO: Support non-read IO events.
    assert(mask == event_io_flag_read);

    reallocate_io_events_if_at_capacity(queue);

    struct pollfd pollfd = {
        .fd = fd,
        .events = POLLIN,
        .revents = 0,
    };

    uint32_t id = queue->next_io_event_id;
    queue->next_io_event_id += 1;

    IoEvent event = {
        .id = id,
        .callback = callback,
        .userdata = userdata,
    };

    queue->io_poll_descriptors[queue->io_events_size] = pollfd;
    queue->io_events[queue->io_events_size] = event;
    queue->io_events_size += 1;

    return (IoEventId){id};
}

void event_queue_remove_io_event(EventQueue* queue, IoEventId id) {
    size_t index;
    if (get_io_event_by_id(queue, id, &index)) {
        remove_io_event_at_position(queue, index);
    }
}

static bool handle_io_events(EventQueue* queue, int timeout_ms) {
    if (queue->io_events_size == 0) {
        return false; // Handled no events, report false.
    }

    int poll_status = poll(queue->io_poll_descriptors, queue->io_events_size, timeout_ms);

    if (poll_status > 0) {
        for (size_t i = 0; i < queue->io_events_size; i++) {
            struct pollfd* polllfd = &queue->io_poll_descriptors[i];

            if ((polllfd->revents & POLLIN) != 0) {
                IoEvent event = queue->io_events[i];
                (*event.callback)(polllfd->fd, event_io_flag_read, event.userdata);
            }

            polllfd->revents = 0;
        }

        return true;
    } else {
        // TODO: Handle poll error and timeout.
        return false;
    }
}

bool event_queue_wait(EventQueue* queue) {
    // TODO: Rewrite wait logic. It's a bit of a mess.

    Timer timer;
    if (!timer_heap_take(&queue->timers, &timer)) {
        int no_timeout = -1;
        return handle_io_events(queue, no_timeout);
    } else if (timer.is_event) {
        // NOTE: Currently, there's no deadline check/wait for events. Since they're 'immediate,'
        // they should always be executed without delay.

        EventId id = (EventId){timer.id};

        size_t index;
        if (get_event_by_id(queue, id, &index)) {
            Event event = queue->events[index];
            (*event.callback)(event.userdata, timer.userdata);
        }

        return true;
    } else {
        uint64_t now_us = time_now_us();

        if (timer.deadline > now_us) {
            int timeout_ms = (timer.deadline - now_us) / 1000;
            handle_io_events(queue, timeout_ms);
        }

        // millisecond granularity of `poll` might not take us up to actual deadline, so sleep
        // again using microsecond deadline:
        time_sleep_until(timer.deadline);

        // Trigger the timer's callback function.
        (*timer.callback)(timer.userdata);

        bool is_periodic = timer.period != TIMER_APERIODIC;
        if (is_periodic) {
            // TODO: This can be optimized by checking if the timer is reinserted before the binary
            // tree sift-down in timer_heap_take.
            timer.deadline += timer.period;
            timer_heap_insert(&queue->timers, timer);
        }

        return true;
    }
}

void event_queue_free(EventQueue* queue) {
    timer_heap_free(&queue->timers);
    free(queue->events);
    free(queue->io_poll_descriptors);
    free(queue->io_events);
}
