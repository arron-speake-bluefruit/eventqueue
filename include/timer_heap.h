#ifndef EVENTQUEUE_TIMER_HEAP_H
#define EVENTQUEUE_TIMER_HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*TimerFunction)(void* userdata);

typedef struct Timer {
    uint64_t deadline;
    TimerFunction function;
    void* userdata;
    uint32_t id;
} Timer;

typedef struct TimerHeap {
    Timer* data;
    size_t size;
    size_t capacity;
} TimerHeap;

TimerHeap timer_heap_new(void);
void timer_heap_insert(TimerHeap* heap, Timer timer);
const Timer* timer_heap_find(const TimerHeap* heap);
bool timer_heap_take(TimerHeap* heap, Timer* out);
void timer_heap_free(TimerHeap* heap);

#endif // EVENTQUEUE_TIMER_HEAP_H
