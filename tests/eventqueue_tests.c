#include "eventqueue.h"
#include "mock_time.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

// --- Utility & mocks --- //

static size_t timer_a_callback_call_count;
static void* timer_a_callback_userdata;
static void timer_a_callback(void* userdata) {
    timer_a_callback_call_count += 1;
    timer_a_callback_userdata = userdata;
}

static size_t timer_b_callback_call_count;
static void timer_b_callback(void* userdata) {
    (void)userdata;
    timer_b_callback_call_count += 1;
}

static size_t event_io_function_call_count;
static int event_io_function_fd;
static EventIoFlag event_io_function_flag;
static void* event_io_function_userdata;
static void event_io_function(int fd, EventIoFlag flag, void* userdata) {
    event_io_function_fd = fd;
    event_io_function_flag = flag;
    event_io_function_userdata = userdata;
    event_io_function_call_count += 1;

    // empty out the available data
    char data;
    while (read(fd, &data, 1) == 1) {}
}

// --- Tests --- //

static void can_add_timers(void) {
    EventQueue queue = event_queue_new();

    int example_value = 0;

    TimerId id_a = event_queue_add_timer(&queue, 3000, timer_a_callback, &example_value);
    TimerId id_b = event_queue_add_timer(&queue, 5000, timer_a_callback, NULL);
    assert(id_a.id != id_b.id);

    assert(timer_a_callback_call_count == 0);

    assert(event_queue_wait(&queue));
    assert(mock_time_get() == 3000);
    assert(timer_a_callback_call_count == 1);
    assert(timer_a_callback_userdata == &example_value);

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

static void can_remove_timers(void) {
    EventQueue queue = event_queue_new();

    TimerId timer = event_queue_add_periodic_timer(&queue, 2000, 2000, timer_a_callback, NULL);

    event_queue_remove_timer(&queue, timer);

    assert(!event_queue_wait(&queue));

    event_queue_free(&queue);
}

static size_t event_callback_call_count;
static void* event_callback_userdata;
static void* event_callback_eventdata;
void event_callback(void* userdata, void* eventdata) {
    event_callback_call_count += 1;
    event_callback_userdata = userdata;
    event_callback_eventdata = eventdata;
}

static void can_add_events(void) {
    EventQueue queue = event_queue_new();

    int a_data = 0;
    int b_data = 0;
    int arg_data = 0;

    EventId event_a = event_queue_add_event(&queue, event_callback, &a_data);
    EventId event_b = event_queue_add_event(&queue, event_callback, &b_data);
    assert(event_a.id != event_b.id);

    event_queue_trigger_event(&queue, event_a, &arg_data);
    assert(event_callback_call_count == 0);

    event_queue_trigger_event(&queue, event_b, &arg_data);
    assert(event_callback_call_count == 0);

    assert(event_queue_wait(&queue));
    assert(event_callback_call_count == 1);
    assert(event_callback_userdata == &a_data);
    assert(event_callback_eventdata == &arg_data);

    assert(event_queue_wait(&queue));
    assert(event_callback_call_count == 2);
    assert(event_callback_userdata == &b_data);
    assert(event_callback_eventdata == &arg_data);

    event_queue_remove_event(&queue, event_a);
    event_queue_remove_event(&queue, event_b);

    // When events are removed, no function call made
    assert(!event_queue_wait(&queue));
    assert(event_callback_call_count == 2);

    event_queue_free(&queue);
}

static void can_add_io_read_event(void) {
    // Set up pipes for testing instead of file descriptors of on-disk files.
    // The pipes are made non-blocking.
    int pipes[2];
    assert(pipe(pipes) == 0);
    int read_pipe = pipes[0];
    int write_pipe = pipes[1];
    assert(fcntl(read_pipe, F_SETFL, O_NONBLOCK) == 0);

    int userdata = 0;
    EventQueue queue = event_queue_new();
    IoEventId event = event_queue_add_io_event(
        &queue, read_pipe, io_event_kind_read, event_io_function, &userdata);

    assert(write(write_pipe, "Hello!\n", 7) == 7);

    assert(event_queue_wait(&queue));
    assert(event_io_function_fd == read_pipe);
    assert(event_io_function_flag == io_event_kind_read);
    assert(event_io_function_userdata == &userdata);
    assert(event_io_function_call_count == 1);

    event_queue_remove_io_event(&queue, event);

    assert(!event_queue_wait(&queue));
    assert(event_io_function_call_count == 1);

    event_queue_free(&queue);
    close(write_pipe);
    close(read_pipe);
}

// --- Test runner -- //

static void setup(void) {
    timer_a_callback_call_count = 0;
    timer_b_callback_call_count = 0;
    timer_a_callback_userdata = NULL;
    event_callback_call_count = 0;
    event_callback_userdata = NULL;
    event_callback_eventdata = NULL;
    event_io_function_fd = 0;
    event_io_function_flag = 0;
    event_io_function_userdata = NULL;
    event_io_function_call_count = 0;
    mock_time_reset();
}

int main(void) {
    void (*tests[])(void) = {
        can_add_timers,
        can_add_periodic_timers,
        can_remove_timers,
        can_add_events,
        can_add_io_read_event,
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < test_count; i++) {
        setup();
        tests[i]();
    }
}
