#include "eventqueue.h"
#include <stdio.h>

static void after_two_second(void* userdata) {
    int* value = (int*)userdata;
    printf("After two seconds, changing from %i to 50\n", *value);
    *value = 50;
}

static void after_three_second(void* userdata) {
    int* value = (int*)userdata;
    printf("After three seconds, changing from %i to 25\n", *value);
    *value = 25;
}

static void every_second(void* userdata) {
    unsigned* counter = (unsigned*)userdata;
    *counter += 1;
    printf("A second has passed\n");
}

static void on_event(void* general_info, void* specific_info) {
    (void)general_info;
    (void)specific_info;
    printf("Event fired\n");
}

int main(void) {
    // Create a new event queue.
    EventQueue queue = event_queue_new();

    // Example data to pass into timer callbacks.
    int value = 100;
    unsigned second_counter = 0;

    // Register two events which fire once 2s and 3s in the future.
    event_queue_add_timer(&queue, 2000000, after_two_second, &value);
    event_queue_add_timer(&queue, 3000000, after_three_second, &value);
    EventId event = event_queue_add_event(&queue, on_event, &second_counter);
    TimerId timer = event_queue_add_periodic_timer(&queue, 500000, 1000000, every_second, &second_counter);

    printf("Value is %i\n", value);

    // Wait until there're no more events.
    while (event_queue_wait(&queue)) {
        if (second_counter == 10) {
            // After 10 calls of `every_second`, stop the timer.
            event_queue_remove_timer(&queue, timer);
            second_counter += 1;
            event_queue_trigger_event(&queue, event, NULL);
        }
    }

    printf("Value is %i\n", value);

    // Clean up the event queue.
    event_queue_free(&queue);
}
