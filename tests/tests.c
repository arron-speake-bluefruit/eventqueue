#include "eventqueue.h"
#include <stddef.h>
#include <assert.h>

// --- Utility & mocks --- //

static size_t timer_callback_call_count;
static void timer_callback(void* userdata) {
    (void)userdata;
    timer_callback_call_count += 1;
}

// --- Tests --- //

static void can_add_timers(void) {
    EventQueue queue = event_queue_new();

    TimerId id_a = event_queue_add_timer(&queue, 3000, timer_callback, NULL);
    TimerId id_b = event_queue_add_timer(&queue, 5000, timer_callback, NULL);
    assert(id_a.id != id_b.id);

    event_queue_free(&queue);
}

// --- Test runner -- //

static void setup(void) {
    timer_callback_call_count = 0;
}

int main(void) {
    void (*tests[])(void) = {
        can_add_timers,
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < test_count; i++) {
        setup();
        tests[i]();
    }
}
