#include "eventqueue.h"

EventQueue event_queue_new(void) {
    return (EventQueue){0};
}

void event_queue_free(EventQueue* queue) {
    (void)queue;
}
