#include "eventqueue.h"

static void can_initialize_event_queue(void) {
    EventQueue queue = event_queue_new();
    event_queue_free(&queue);
}

int main(void) {
    can_initialize_event_queue();
}
