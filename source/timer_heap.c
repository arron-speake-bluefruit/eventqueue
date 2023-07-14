#include "timer_heap.h"
#include <stdlib.h>

static void swap_elements(TimerHeap* heap, size_t a, size_t b) {
    Timer swap = heap->data[a];
    heap->data[a] = heap->data[b];
    heap->data[b] = swap;
}

// If node `index` has children return true and store the smaller child index in `out`. Otherwise,
// return false.
static bool get_minimal_child(const TimerHeap* heap, size_t index, size_t* out) {
    size_t left_index = (2 * index) + 1;
    size_t right_index = (2 * index) + 2;

    if (right_index < heap->size) {
        size_t left_deadline = heap->data[left_index].deadline;
        size_t right_deadline = heap->data[right_index].deadline;

        if (left_deadline < right_deadline) {
            *out = left_index;
            return true;
        } else {
            *out = right_index;
            return true;
        }
    } else if (left_index < heap->size) {
        *out = left_index;
        return true;
    } else {
        return false;
    }
}

static void sift_down(TimerHeap* heap, size_t root_index) {
    // find minimal child
    size_t child_index;
    if (!get_minimal_child(heap, root_index, &child_index)) {
        return; // No children. Sift-down done.
    }

    uint64_t child_deadline = heap->data[child_index].deadline;
    uint64_t root_deadline = heap->data[root_index].deadline;

    if (root_deadline > child_deadline) {
        swap_elements(heap, root_index, child_index);
        sift_down(heap, child_index);
    }
}

static void sift_up(TimerHeap* heap, size_t index) {
    if (index != 0) {
        size_t parent_index = (index - 1) / 2;

        uint64_t this_deadline = heap->data[index].deadline;
        uint64_t parent_deadline = heap->data[parent_index].deadline;

        if (parent_deadline > this_deadline) {
            swap_elements(heap, index, parent_index);
            sift_up(heap, parent_index);
        }
    }
}

TimerHeap timer_heap_new(void) {
    Timer* data = malloc(sizeof(Timer));
    if (data == NULL) abort();

    return (TimerHeap){
        .data = data,
        .capacity = 1,
        .size = 0,
    };
}

static void reallocate_if_at_capacity(TimerHeap* heap) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->data = realloc(heap->data, sizeof(Timer) * heap->capacity);
        if (heap->data == NULL) abort();
    }
}

static size_t append_element_unchecked(TimerHeap* heap, Timer timer) {
    size_t index = heap->size;

    heap->data[index] = timer;
    heap->size += 1;

    return index;
}

static bool get_timer_index_by_id(const TimerHeap* heap, TimerId id, size_t* out) {
    for (size_t i = 0; i < heap->size; i++) {
        if (heap->data[i].id == id.id) {
            *out = i;
            return true;
        }
    }

    return false;
}

void timer_heap_insert(TimerHeap* heap, Timer timer) {
    reallocate_if_at_capacity(heap);
    size_t index = append_element_unchecked(heap, timer);
    sift_up(heap, index);
}

const Timer* timer_heap_find(const TimerHeap* heap) {
    if (heap->size == 0) {
        return NULL;
    } else {
        return &heap->data[0];
    }
}

bool timer_heap_take(TimerHeap* heap, Timer* out) {
    if (heap->size == 0) {
        return false;
    } else {
        // Extract data
        *out = heap->data[0];

        // Replace root with last element of the last level
        heap->data[0] = heap->data[heap->size - 1];
        heap->size -= 1;

        // Sort the heap so the root is minimal.
        sift_down(heap, 0);

        return true;
    }
}

void timer_heap_remove_id(TimerHeap* heap, TimerId id) {
    size_t index;
    if (get_timer_index_by_id(heap, id, &index)) {
        // Replace to-be-removed timer with least-minimal element.
        heap->data[index] = heap->data[heap->size - 1];
        heap->size -= 1;

        // Sort the heap so the root is minimal.
        sift_down(heap, index);
    }
}

void timer_heap_free(TimerHeap* heap) {
    free(heap->data);
}
