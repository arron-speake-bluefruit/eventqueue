#include "eventqueue.h"
#include "mock_time.h"
#include <assert.h>
#include <string.h>

// --- Utility --- //

static uint64_t hash64(uint64_t n) {
  n ^= n >> 33;
  n *= 0xff51afd7ed558ccdL;
  n ^= n >> 33;
  n *= 0xc4ceb9fe1a85ec53L;
  n ^= n >> 33;
  return n;
}

// --- Tests --- //

static void new_timer_heap_is_empty(void) {
    TimerHeap heap = timer_heap_new();

    const Timer* timer_in_empty_heap = timer_heap_find(&heap);
    assert(timer_in_empty_heap == NULL);

    Timer timer_out = {0};
    bool took_from_empty_heap = timer_heap_take(&heap, &timer_out);
    assert(!took_from_empty_heap);

    timer_heap_free(&heap);
}

static void can_remove_timer_after_inserting_it(void) {
    TimerHeap heap = timer_heap_new();

    Timer timer = {0};
    timer_heap_insert(&heap, timer);

    const Timer* timer_ptr = timer_heap_find(&heap);
    assert(timer_ptr != NULL);
    assert(memcmp(&timer, timer_ptr, sizeof(Timer)) == 0);

    Timer timer_out = {0};
    bool got_timer = timer_heap_take(&heap, &timer_out);
    assert(got_timer);
    assert(memcmp(&timer, &timer_out, sizeof(Timer)) == 0);

    // After `take`, heap is empty
    assert(timer_heap_find(&heap) == NULL);

    timer_heap_free(&heap);
}

static void insertion_of_multiple_timers_maintains_ordering(void) {
    TimerHeap heap = timer_heap_new();

    timer_heap_insert(&heap, (Timer){ .deadline = 2, .id = 0 });
    timer_heap_insert(&heap, (Timer){ .deadline = 4, .id = 1 });
    timer_heap_insert(&heap, (Timer){ .deadline = 1, .id = 2 });
    timer_heap_insert(&heap, (Timer){ .deadline = 3, .id = 3 });

    Timer first = {0};
    assert(timer_heap_take(&heap, &first));
    assert(first.deadline == 1);
    assert(first.id == 2);

    Timer second = {0};
    assert(timer_heap_take(&heap, &second));
    assert(second.deadline == 2);
    assert(second.id == 0);

    Timer third = {0};
    assert(timer_heap_take(&heap, &third));
    assert(third.deadline == 3);
    assert(third.id == 3);

    Timer fourth = {0};
    assert(timer_heap_take(&heap, &fourth));
    assert(fourth.deadline == 4);
    assert(fourth.id == 1);

    // Heap is now empty.
    assert(timer_heap_find(&heap) == NULL);

    timer_heap_free(&heap);
}

static void large_number_of_timers_are_well_ordered_in_heap(void) {
    TimerHeap heap = timer_heap_new();

    const size_t element_count = 100000;

    for (size_t i = 0; i < element_count; i++) {
        // Hash the index for the deadline. This gives you a consistent pseudorandom sequence of
        // deadlines. (No stdlib rand in tests!!)
        uint64_t deadline = hash64(i + 1);
        Timer timer = {
            .deadline = deadline,
        };
        timer_heap_insert(&heap, timer);
    }

    uint64_t last_deadline = 0;
    Timer timer = {0};
    while (timer_heap_take(&heap, &timer)) {
        assert(timer.deadline >= last_deadline);
        last_deadline = timer.deadline;
    }

    timer_heap_free(&heap);
}

static void removing_a_timer_id_removes_timer_from_heap(void) {
    TimerHeap heap = timer_heap_new();

    timer_heap_insert(&heap, (Timer){ .deadline = 100, .id = 1 });
    timer_heap_insert(&heap, (Timer){ .deadline = 200, .id = 2 });
    timer_heap_insert(&heap, (Timer){ .deadline = 300, .id = 3 });

    timer_heap_remove_id(&heap, (TimerId){1});

    Timer first;
    assert(timer_heap_take(&heap, &first));
    assert(first.id == 2);

    Timer second;
    assert(timer_heap_take(&heap, &second));
    assert(second.id == 3);

    Timer third;
    assert(!timer_heap_take(&heap, &third));

    timer_heap_free(&heap);
}

int main(void) {
    void (*tests[])(void) = {
        new_timer_heap_is_empty,
        can_remove_timer_after_inserting_it,
        insertion_of_multiple_timers_maintains_ordering,
        large_number_of_timers_are_well_ordered_in_heap,
        removing_a_timer_id_removes_timer_from_heap,
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < test_count; i++) {
        tests[i]();
    }
}
