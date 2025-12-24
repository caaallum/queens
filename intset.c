#include "intset.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint32_t hash_u32(uint32_t x) {
    // decent integer hash
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static int next_pow2(int x) {
    int p = 1;
    while (p < x) p <<= 1;
    return p;
}

bool intset_init(intset_t* s, int expected_count) {
    if (!s) return false;
    int cap = next_pow2(expected_count * 2 + 8); // load factor ~0.5
    s->keys = (int*)malloc(sizeof(int) * cap);
    s->used = (unsigned char*)malloc(sizeof(unsigned char) * cap);
    if (!s->keys || !s->used) {
        free(s->keys); free(s->used);
        return false;
    }
    memset(s->used, 0, cap);
    s->cap = cap;
    s->size = 0;
    return true;
}

void intset_destroy(intset_t* s) {
    if (!s) return;
    free(s->keys);
    free(s->used);
    s->keys = NULL;
    s->used = NULL;
    s->cap = 0;
    s->size = 0;
}

bool intset_insert(intset_t* s, int key) {
    // returns true if inserted new, false if already present
    uint32_t h = hash_u32((uint32_t)key);
    int mask = s->cap - 1;
    int idx = (int)(h & (uint32_t)mask);

    for (;;) {
        if (!s->used[idx]) {
            s->used[idx] = 1;
            s->keys[idx] = key;
            s->size++;
            return true;
        }
        if (s->keys[idx] == key) {
            return false;
        }
        idx = (idx + 1) & mask;
    }
}