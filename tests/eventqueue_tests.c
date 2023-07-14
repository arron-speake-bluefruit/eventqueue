#include "eventqueue.h"
#include "mock_time.h"
#include <assert.h>

// --- Utility & mocks --- //

static size_t timer_a_callback_call_count;
static void timer_a_callback(void* userdata) {
    (void)userdata;
    timer_a_callback_call_count += 1;
}

static size_t timer_b_callback_call_count;
static void timer_b_callback(void* userdata) {
    (void)userdata;
    timer_b_callback_call_count += 1;
}

// --- Tests --- //

static void can_add_timers(void) {
    EventQueue queue = event_queue_new();

    TimerId id_a = event_queue_add_timer(&queue, 3000, timer_a_callback, NULL);
    TimerId id_b = event_queue_add_timer(&queue, 5000, timer_a_callback, NULL);
    assert(id_a.id != id_b.id);

    assert(timer_a_callback_call_count == 0);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 3000);
    assert(timer_a_callback_call_count == 1);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 5000);
    assert(timer_a_callback_call_count == 2);

    // No event to wait for, so immediate return.
    assert(!event_queue_wait(&queue));
    assert(mock_time_get() == 5000);
    assert(timer_a_callback_call_count == 2);

    event_queue_free(&queue);
}

static void can_add_periodic_timers(void) {
    EventQueue queue = event_queue_new();

    TimerId id_a = event_queue_add_periodic_timer(&queue, 2000, 2000, timer_a_callback, NULL);
    TimerId id_b = event_queue_add_periodic_timer(&queue, 3000, 3000, timer_b_callback, NULL);
    assert(id_a.id != id_b.id);

    assert(timer_a_callback_call_count == 0);
    assert(timer_b_callback_call_count == 0);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 2000);
    assert(timer_a_callback_call_count == 1);
    assert(timer_b_callback_call_count == 0);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 3000);
    assert(timer_a_callback_call_count == 1);
    assert(timer_b_callback_call_count == 1);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 4000);
    assert(timer_a_callback_call_count == 2);
    assert(timer_b_callback_call_count == 1);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 6000);
    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 6000);
    assert(timer_a_callback_call_count == 3);
    assert(timer_b_callback_call_count == 2);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 8000);
    assert(timer_a_callback_call_count == 4);
    assert(timer_b_callback_call_count == 2);

    event_queue_free(&queue);
}

// --- Test runner -- //

static void setup(void) {
    timer_a_callback_call_count = 0;
    timer_b_callback_call_count = 0;
    mock_time_reset();
}

int main(void) {
    void (*tests[])(void) = {
        can_add_timers,
        can_add_periodic_timers,
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < test_count; i++) {
        setup();
        tests[i]();
    }
}
