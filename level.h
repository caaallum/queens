#ifndef __LEVEL_H
#define __LEVEL_H

typedef struct {
	int rows;
	int cols;
	int* regions;
} level_t;

level_t* level_generate(int rows, int cols);

#endif /* __LEVEL_H */
