/* gui/button.c */
extern void gui_button_draw(widget_st *widget);
/* gui/card.c */
extern void card_deck_init(card_t *deck, int n);
extern void card_deck_shuffle(card_t *deck, int n);
extern int card_pile_cascade_step(card_game_st *game, card_pile_st *p);
extern int card_pile_top_y(card_game_st *game, card_pile_st *p);
extern card_t card_pile_pop(card_pile_st *p);
extern void card_pile_push(card_pile_st *p, card_t c);
extern void card_pile_uncover_top(card_pile_st *pile);
extern void card_draw(card_game_st *game, int x, int y, card_t card, int is_sel);
extern void card_stub_draw(card_game_st *game, int x, int y, int height, card_t card);
extern void card_back_draw(card_game_st *game, int x, int y);
extern void card_back_stub_draw(card_game_st *game, int x, int y, int height);
extern void card_pile_draw(card_game_st *game, card_pile_st *p);
extern void card_cursor_draw(card_game_st *game, int visible);
extern void card_game_exec_cur_move(card_game_st *game);
/* gui/grid.c */
extern void gui_grid_rect(grid_st *grid, rect_st *out);
extern void gui_grid_cell_rect(grid_st *grid, int col, int row, rect_st *out);
extern void gui_grid_draw_background(grid_st *grid, window_st *window, uint8_t color);
/* gui/main.c */
extern rect_st gui_app_rect;
extern app_st *gui_current_app;
extern int gui_colors_inverted;
extern uint8_t gui_color_bg;
extern uint8_t gui_color_fg;
extern void gui_set_colors_inverted(int inverted);
extern void gui_run_app(app_st *app);
extern void gui_main(void);
/* gui/rect.c */
extern const point_st GUI_POINT_ZERO;
extern const rect_st GUI_RECT_SCREEN;
extern void gui_rect_copy(rect_st *dst, const rect_st *src);
extern void gui_point_copy(point_st *dst, const point_st *src);
extern void gui_size_copy(size_st *dst, const size_st *src);
extern int gui_rect_is_empty(const rect_st *r);
extern uint16_t gui_rect_area(const rect_st *r);
extern int gui_rect_touches(const rect_st *a, const rect_st *b);
extern void gui_rect_init(rect_st *out, int x, int y, int width, int height);
extern void gui_rect_translate(rect_st *r, const point_st *v);
extern void gui_rect_center(rect_st *r, const rect_st *container);
extern void gui_rect_shrink(rect_st *r, int amount);
extern void gui_rect_enclose(rect_st *a, const rect_st *b);
extern void gui_rect_clip(rect_st *r, const rect_st *clipper);
extern const char * gui_rect_format(const rect_st *r);
/* gui/status.c */
extern void gui_status_set_tl(const char *fmt, ...);
extern void gui_status_set_tr(const char *fmt, ...);
extern void gui_status_set(const char *fmt, ...);
extern void gui_status_set_urgent(const char *fmt, ...);
extern void gui_status_set_br(const char *fmt, ...);
extern void gui_status_init(void);
/* gui/surface.c */
extern void gui_surface_init(void);
extern void gui_surface_clear(void);
extern void gui_surface_invert(void);
extern void gui_surface_mark_dirty(const point_st *origin, const rect_st *rect);
extern void gui_surface_flush(void);
extern void gui_surface_draw_pixel(const point_st *origin, int x, int y, uint8_t color);
extern void gui_surface_draw_h_seg(const point_st *origin, int x, int y, int w, uint8_t color);
extern void gui_surface_draw_v_seg(const point_st *origin, int x, int y, int h, uint8_t color);
extern void gui_surface_draw_border(const point_st *origin, const rect_st *r, uint8_t color);
extern void gui_surface_draw_rect(const point_st *origin, const rect_st *rect, uint8_t color);
extern void gui_surface_draw_char(const point_st *origin, uint16_t x, uint16_t y, font_st *font, uint8_t ch, uint8_t fg, uint8_t bg);
extern void gui_surface_draw_str(const point_st *origin, uint16_t x, uint16_t y, font_st *font, const char *s, uint8_t fg, uint8_t bg);
extern void gui_surface_draw_str_lines(const point_st *origin, uint16_t x, uint16_t y, uint8_t line_spc, font_st *font, const char **lines, uint8_t fg, uint8_t bg);
extern void gui_surface_draw_str_centered(const point_st *origin, const rect_st *rect, font_st *font, const char *s, uint8_t fg, uint8_t bg);
extern void gui_surface_draw_bitmap(const point_st *origin, const size_st *bounds, int dst_x, int dst_y, bitmap_st *bitmap, uint8_t fill);
extern void gui_surface_draw_bitmap_centered(const point_st *origin, const size_st *bounds, const rect_st *rect, bitmap_st *bitmap, uint8_t fill);
extern void gui_surface_draw_dots_pattern(const point_st *origin, const rect_st *rect);
/* gui/window.c */
extern void gui_window_init(window_st *window, int width, int height);
extern void gui_window_draw(window_st *window, uint8_t bg_color, int border);
