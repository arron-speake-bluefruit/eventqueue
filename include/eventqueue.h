#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

typedef struct EventQueue {
    int dummy;
} EventQueue;

EventQueue event_queue_new(void);
void event_queue_free(EventQueue* queue);

#endif // EVENT_QUEUE_H
