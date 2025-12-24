#ifndef __GRID_H
#define __GRID_H

#include "level.h"

#include <SDL3/SDL.h>

typedef struct grid_t grid_t;

grid_t* grid_create(const level_t const *level, float cell_size);

void grid_handle_event(grid_t* grid, SDL_Event* event);

bool grid_check_win(const grid_t const* grid);

void grid_draw(const grid_t* grid, SDL_Renderer* renderer);

#endif /* __GRID_H */