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
    (void)userdata;
    printf("A second has passed\n");
}

int main(void) {
    // Create a new event queue.
    EventQueue queue = event_queue_new();

    // Example data to pass into timer callbacks.
    int value = 100;

    // Register two events which fire once 2s and 3s in the future.
    event_queue_add_timer(&queue, 2000000, after_two_second, &value);
    event_queue_add_timer(&queue, 3000000, after_three_second, &value);
    event_queue_add_periodic_timer(&queue, 500000, 1000000, every_second, &value);

    printf("Value is %i\n", value);

    while (event_queue_wait(&queue)) {
        // Wait until there're no more events.
    }

    printf("Value is %i\n", value);

    // Clean up the event queue.
    event_queue_free(&queue);
}
