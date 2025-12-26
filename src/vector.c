#include "vector.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define VECTOR_INIT_CAPACITY 4

struct vector_t {
    void** items;
    int capacity;
    int total;
};

vector_t*
vector_new(void) {
    vector_t* vector = malloc(sizeof(vector_t));
    assert(vector);

    vector->capacity = VECTOR_INIT_CAPACITY;
    vector->total = 0;
    vector->items = malloc(sizeof(void*) * vector->capacity);
    assert(vector->items);

    return vector;
}

vector_t*
vector_new_size(int size) {
    vector_t* vector = malloc(sizeof(vector_t));
    assert(vector);

    vector->capacity = size;
    vector->total = 0;
    vector->items = malloc(sizeof(void*) * vector->capacity);
    assert(vector->items);

    return vector;
}

void
vector_destroy(vector_t* vector) {
    assert(vector);

    free(vector->items);
    free(vector);
}

int
vector_size(vector_t* vector) {
    assert(vector);
    return vector->total;
}

void
vector_shuffle(vector_t* vector) {
    if (!vector || vector->total <= 1)
        return;

    for (int i = vector->total - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);

        void* tmp = vector->items[i];
        vector->items[i] = vector->items[j];
        vector->items[j] = tmp;
    }
}

static
void
vector_resize(vector_t* vector, int capacity) {
    void** items = realloc(vector->items, sizeof(void*) * capacity);
    assert(items);

    vector->items = items;
    vector->capacity = capacity;
}

void
vector_add(vector_t* vector, void* item) {
    assert(vector);
    assert(item);

    if (vector->total == vector->capacity) {
        vector_resize(vector, vector->capacity * 2);
    }

    vector->items[vector->total++] = item;
}

void 
vector_set(vector_t* vector, void* item, int pos) {
    assert(vector);
    assert(item);

    vector->items[pos] = item;
}

void
vector_pushback(vector_t* vector, void* item, int size) {
    assert(vector);
    assert(item);
    assert(size > 0);

    if (vector->total == vector->capacity) {
        vector_resize(vector, vector->capacity * 2);
    }

    void* item_ptr = malloc(sizeof(size));
    assert(item_ptr);
    memcpy(item_ptr, item, size);

    vector->items[vector->total++] = item_ptr;
}

void 
vector_popback(vector_t* vector) {
    vector_delete(vector, vector->total);
}

void
vector_delete(vector_t* vector, int index) {
    assert(vector);
    if (index < 0 || index >= vector->total) {
        return;
    }

    vector->items[index] = NULL;

    for (int i = index; i < vector->total - 1; i++) {
        vector->items[i] = vector->items[i + 1];
        vector->items[i + 1] = NULL;
    }

    vector->total--;

    if (vector->total > 0 && vector->total == vector->capacity / 4) {
        vector_resize(vector, vector->capacity / 2);
    }
}

void*
vector_get(vector_t* vector, int index) {
    assert(vector);

    if (index >= 0 && index < vector->total) {
        return vector->items[index];
    }
    return NULL;
}