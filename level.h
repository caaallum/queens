#ifndef __LEVEL_H
#define __LEVEL_H

typedef struct {
	int rows;
	int cols;
	int* regions;
} level_t;

level_t* level_generate(int rows, int cols);

void level_destroy(level_t* level);

#endif /* __LEVEL_H */
