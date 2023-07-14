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

int main(void) {
    EventQueue queue = event_queue_new();

    int value = 100;

    event_queue_add_timer(&queue, 2000000, after_two_second, &value);
    event_queue_add_timer(&queue, 3000000, after_three_second, &value);

    printf("Value is %i\n", value);

    // TODO: Replace with loop.
    event_queue_wait(&queue);
    event_queue_wait(&queue);

    printf("Value is %i\n", value);

    event_queue_free(&queue);
}
