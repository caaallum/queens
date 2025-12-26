#ifndef __INTSET_H
#define __INTSET_H

#include <stdbool.h>

typedef struct intset {
	int* keys;
	unsigned char* used;
	int cap;
	int size;
} intset_t;

bool intset_init(intset_t* inset, int expected_count);

void intset_destroy(intset_t* s);

bool intset_insert(intset_t* s, int key);

#endif /* __INTSET_H */
