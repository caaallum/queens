#include "grid.h"

#include "vector.h"
#include "intset.h"

#include <malloc.h>
#include <assert.h>
#include <stdio.h>

typedef struct {
	int r, g, b;
} color_t;

typedef enum {
	CELL_EMPTY,
	CELL_QUEEN,
	CELL_PLUS
} cell_state_t;

typedef struct {
	int region;
	color_t color;
	cell_state_t state;
} cell_t;

typedef enum { DRAG_NONE, DRAG_PAINT_PLUS, DRAG_ERASE_PLUS } drag_mode_t;

struct grid_t {
	int rows;
	int cols;
	float cell_size;
	vector_t* cells; // cell_t
	bool left_mouse_down;
	int last_r, last_c;
	drag_mode_t drag_mode;
};

static
void
_region_colour(int id, color_t* out_color) {
	static color_t palette[] = {
		{220,20,60},{60,80,200},{60,180,90},
		{200,200,60},{180,60,180},{60,180,180}
	};
	int n = sizeof(palette) / sizeof(color_t);
	*out_color = palette[id % n];
}

static
bool
_can_place_queen(const grid_t* grid, int row, int col) {
	cell_t* t = (cell_t*)vector_get(grid->cells, row * grid->cols + col);

	// Only one queen per region
	V_FOREACH(grid->cells, cell_t, cell, cell_index) {
		if (cell->state == CELL_QUEEN && cell->region == t->region) {
			return false;
		}
	}

	// Only one queen per row
	for (int i = 0; i < grid->rows; ++i) {
		cell_t* cell = (cell_t*)vector_get(grid->cells, i * grid->cols + col);
		if (cell->state == CELL_QUEEN) {
			return false;
		}
	}

	// Only one queen per column
	for (int i = 0; i < grid->cols; ++i) {
		cell_t* cell = (cell_t*)vector_get(grid->cells, row * grid->cols + i);
		if (cell->state == CELL_QUEEN) {
			return false;
		}
	}

	// Only one queen adjacent (including diagonals)
	for (int dr = -1; dr <= 1; ++dr) {
		for (int dc = -1; dc <= 1; ++dc) {
			if (dr == 0 && dc == 0) {
				continue;
			}
			int nr = row + dr;
			int nc = col + dc;
			if (nr >= 0 && nc >= 0 && nr < grid->rows && nc < grid->cols) {
				cell_t* cell = (cell_t*)vector_get(grid->cells, nr * grid->cols + nc);
				if (cell->state == CELL_QUEEN) {
					return false;
				}
			}
		}
	}

	return true;
}

grid_t* 
grid_create(const level_t const* level, float cell_size) {
	grid_t* grid = (grid_t*)malloc(sizeof(grid_t));
	assert(grid);

	grid_reset(grid, level, cell_size);

	return grid;
}

void
grid_reset(grid_t* grid, const level_t const* level, float cell_size) {
	grid->rows = level->rows;
	grid->cols = level->cols;
	grid->cell_size = cell_size;

	grid->left_mouse_down = false;
	grid->drag_mode = DRAG_NONE;
	grid->last_r = -1;
	grid->last_c = -1;

	if (grid->cells) {
		V_FOREACH(grid->cells, cell_t, cell, c_i) {
			free(cell);
		}
		vector_destroy(grid->cells);
	}

	grid->cells = vector_new(); // cell_t

	int size = grid->rows * grid->cols;
	for (int i = 0; i < size; ++i) {
		cell_t* cell = (cell_t*)malloc(sizeof(cell_t));
		assert(cell);
		cell->region = level->regions[i];
		_region_colour(cell->region, &cell->color);
		cell->state = CELL_EMPTY;
		vector_add(grid->cells, cell);
	}
}

static void
_apply_left_drag(grid_t* grid, int r, int c) {
	cell_t* cell = (cell_t*)vector_get(grid->cells, r * grid->cols + c);

	if (cell->state == CELL_QUEEN) return;

	if (grid->drag_mode == DRAG_PAINT_PLUS) {
		cell->state = CELL_PLUS;
	}
	else if (grid->drag_mode == DRAG_ERASE_PLUS) {
		if (cell->state == CELL_PLUS) cell->state = CELL_EMPTY;
	}
}

void
grid_handle_event(grid_t* grid, SDL_Event* event) {
	switch (event->type) {

		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			int c = event->button.x / grid->cell_size;
			int r = event->button.y / grid->cell_size;

			if (r < 0 || r >= grid->rows || c < 0 || c >= grid->cols) break;

			if (event->button.button == SDL_BUTTON_LEFT) {
				grid->left_mouse_down = true;

				// Determine drag intent based on what we clicked
				cell_t* cell = (cell_t*)vector_get(grid->cells, r * grid->cols + c);
				if (cell->state == CELL_QUEEN) break;

				// Single-click toggle still works:
				bool was_plus = (cell->state == CELL_PLUS);
				cell->state = was_plus ? CELL_EMPTY : CELL_PLUS;

				// Drag mode follows what the click just did
				grid->drag_mode = was_plus ? DRAG_ERASE_PLUS : DRAG_PAINT_PLUS;

				// IMPORTANT: prevent the first motion from re-processing same cell
				grid->last_r = r;
				grid->last_c = c;
				break;
			}

			if (event->button.button == SDL_BUTTON_RIGHT) {
				cell_t* cell = (cell_t*)vector_get(grid->cells, r * grid->cols + c);
				if (cell->state == CELL_QUEEN) {
					cell->state = CELL_EMPTY;
				}
				else {
					if (_can_place_queen(grid, r, c)) {
						cell->state = CELL_QUEEN;
					}
				}
				break;
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event->button.button == SDL_BUTTON_LEFT) {
				grid->left_mouse_down = false;
				grid->drag_mode = DRAG_NONE;
			}
			break;

		case SDL_EVENT_MOUSE_MOTION: {
			if (!grid->left_mouse_down) break;

			int c = event->motion.x / grid->cell_size;
			int r = event->motion.y / grid->cell_size;

			if (r < 0 || r >= grid->rows || c < 0 || c >= grid->cols) break;

			if (r == grid->last_r && c == grid->last_c) break;

			grid->last_r = r;
			grid->last_c = c;

			_apply_left_drag(grid, r, c);
			break;
		}

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			grid->left_mouse_down = false;
			grid->drag_mode = DRAG_NONE;
			break;
	}
}

bool
grid_check_win(const grid_t const* grid) {
	intset_t regions;
	intset_t regions_with_queen;
	int cell_count = (int)vector_size(grid->cells);

	if (!intset_init(&regions, cell_count)) return false;
	if (!intset_init(&regions_with_queen, cell_count)) {
		intset_destroy(&regions);
		return false;
	}

	int queen_count = 0;

	V_FOREACH(grid->cells, cell_t, cell, cell_index) {
		intset_insert(&regions, cell->region);
		if (cell->state == CELL_QUEEN) {
			queen_count++;
			intset_insert(&regions_with_queen, cell->region);
		}
	}

	bool won = (queen_count > 0) &&
		(queen_count == regions.size) &&
		(queen_count == regions_with_queen.size);

	intset_destroy(&regions_with_queen);
	intset_destroy(&regions);
	return won;
}

void
grid_draw(const grid_t* grid, SDL_Renderer* renderer) {
	for (int r = 0; r < grid->rows; ++r) {
		for (int c = 0; c < grid->cols; ++c) {
			cell_t* cell = (cell_t*)vector_get(grid->cells, r * grid->cols + c);
			SDL_FRect rect = { c * grid->cell_size + 1, r * grid->cell_size + 1, grid->cell_size - 2.f, grid->cell_size - 2.f };
			SDL_SetRenderDrawColor(renderer, cell->color.r, cell->color.g, cell->color.b, 255);
			SDL_RenderFillRect(renderer, &rect);


			if (cell->state == CELL_QUEEN) {
				SDL_FRect queen_rect = { c * grid->cell_size + grid->cell_size * 0.15f,
										 r * grid->cell_size + grid->cell_size * 0.15f,
										 grid->cell_size * 0.7f,
										 grid->cell_size * 0.7f };
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderFillRect(renderer, &queen_rect);
			}

			if (cell->state == CELL_PLUS) {
				float t = grid->cell_size * 0.1f;
				float l = grid->cell_size * 0.6f;
				SDL_FRect h_rect = { c * grid->cell_size + grid->cell_size * 0.2f,
									 r * grid->cell_size + grid->cell_size * 0.5f - t / 2,
									 l, t };
				SDL_FRect v_rect = { c * grid->cell_size + grid->cell_size * 0.5f - t / 2,
									 r * grid->cell_size + grid->cell_size * 0.2f,
									 t, l };
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderFillRect(renderer, &h_rect);
				SDL_RenderFillRect(renderer, &v_rect);
			}
		}
	}
}