// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: grid.c - Grid utilities
// --------------------------------------------------------------------------------------

#include <gui.h>

rect_st
gui_grid_rect(grid_st *grid)
{
    return (rect_st) {
        .x = grid->x,
        .y = grid->y,
        .width = GRID_WIDTH_SPACED(grid->cell_width, grid->cols),
        .height = GRID_HEIGHT_SPACED(grid->cell_height, grid->rows),
    };
}

rect_st
gui_grid_cell_rect(grid_st *grid, int col, int row)
{
    return (rect_st) {
        .x = grid->x + col * grid->cell_width + col,
        .y = grid->y + row * grid->cell_height + row,
        .width = grid->cell_width,
        .height = grid->cell_height,
    };
}

void
gui_grid_draw_background(grid_st *grid, window_st *window, uint8_t color)
{
    rect_st grid_rect = gui_grid_rect(grid);
    gui_surface_draw_rect(window->surface, grid_rect, color);
    gui_wm_render_window_region(window, grid_rect);
}
