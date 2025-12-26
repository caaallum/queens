#include "level.h"
#include "vector.h"
#include "vector2.h"

#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

static
void 
shuffle_int_array(int* arr, int n) {
	if (!arr || n <= 1) return;

	for (int i = n - 1; i > 0; --i) {
		int j = rand() % (i + 1); // random index from 0 to i

		// swap arr[i] and arr[j]
		int tmp = arr[i];
		arr[i] = arr[j];
		arr[j] = tmp;
	}
}

static
level_t*
level_init(int rows, int cols) {
	level_t *level = (level_t*)malloc(sizeof(level_t));
	assert(level);

	level->rows = rows;
	level->cols = cols;

	int size = rows * cols;
	level->regions = (int*)malloc(size * sizeof(int));
	assert(level->regions);
	// Set all regions to -1
	for (int i = 0; i < size; ++i) {
		level->regions[i] = -1;
	}

	return level;
}

static
bool place_row(int row, int rows, int cols, int* columns, vector_t *queens) {
	if (row == rows) {
		return true;
	}

	int* col_order = (int*)malloc(cols * sizeof(int));
	assert(col_order);

	for (int i = 0; i < cols; ++i) {
		col_order[i] = i;
	}
	shuffle_int_array(col_order, cols);

	for (int i = 0; i < cols; ++i) {
		int col = col_order[i];

		if (columns[col]) {
			continue;
		}

		bool adjacent = false;
		V_FOREACH(queens, vector2i, q, q_i) {
			if (abs(q->x - row) <= 1 && abs(q->y - col) <= 1) {
				adjacent = true;
				break;
			}
		}
		if (adjacent) {
			continue;
		}

		columns[col] = 1;
		vector2i queen = { row, col };
		vector_pushback(queens, &queen, sizeof(queen));

		if (place_row(row + 1, rows, cols, columns, queens)) {
			free(col_order);
			return true;
		}

		vector_popback(queens);
		columns[col] = 0;
	}
	
	free(col_order);
	return false;
}

level_t *
level_generate(int rows, int cols) {
	level_t* level = level_init(rows, cols);

	vector_t* queens = vector_new(); // vector2i
	int* columns = (int*)malloc(cols * sizeof(int));
	assert(columns);
	// Set all colums to 0
	for (int i = 0; i < cols; ++i) {
		columns[i] = 0;
	}

	place_row(0, rows, cols, columns, queens);

	// One region per queen
	int queens_size = vector_size(queens);
	for (int i = 0; i < queens_size; ++i) {
		vector2i* q = (vector2i * )vector_get(queens, i);
		int pos = q->x * cols + q->y;
		level->regions[pos] = i;
	}

	// Flood fill
	int size = rows * cols;
	int* queue = (int*)malloc(size * sizeof(int));
	assert(queue);
	int front = 0, back = 0;

	// Push all non -1 indices into the queue
	for (int i = 0; i < size; ++i) {
		if (level->regions[i] != -1) {
			queue[back++] = i;
		}
	}

	int d[5] = { -1, 0, 1, 0, -1 }; // 4-way directions

	while (front < back) {
		int idx = queue[front++];
		int r = idx / cols;
		int c = idx % cols;

		for (int i = 0; i < 4; ++i) {
			int nr = r + d[i];
			int nc = c + d[i + 1];

			if (nr < 0 || nc < 0 || nr >= rows || nc >= cols)
				continue;

			int nidx = nr * cols + nc;
			if (level->regions[nidx] == -1) {
				level->regions[nidx] = level->regions[idx];
				queue[back++] = nidx;
			}
		}
	}

	free(queue);
	free(columns);
	vector_destroy(queens);

	return level;
}

void
level_destroy(level_t* level) {
	free(level->regions);
	free(level);
}