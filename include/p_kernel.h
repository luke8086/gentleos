/* kernel/debug.c */
extern int krn_debug_text_mode_enabled;
extern void (*krn_debug_status_cb)(const char *, ...);
extern void krn_debug_printf(const char *fmt, ...);
extern void krn_debug_assert(int expr, const char *file, unsigned line);
extern void krn_debug_beep_adv(unsigned hz, unsigned msecs, unsigned count);
extern void krn_debug_beep(void);
/* kernel/event.c */
extern void krn_event_wait(event_st *out);
extern int krn_event_ipush(event_st *event);
extern int krn_event_push(event_st *event);
extern int krn_event_pop(event_st *event);
extern uint16_t krn_event_count(void);
/* kernel/heap.c */
extern void far * krn_heap_alloc(uint16_t size);
extern void krn_heap_init(void);
/* kernel/keyboard.c */
extern uint16_t krn_keyboard_getc(void);
extern void krn_keyboard_handle_intr(void);
extern void krn_keyboard_handle_bios(void);
extern void krn_keyboard_init(void);
extern void krn_keyboard_deinit(void);
/* kernel/main.c */
extern isr_st far *krn_ivt;
extern void krn_main(void);
extern void krn_exit(void);
extern void krn_set_isr(uint8_t no, uint16_t seg, uint16_t ofs);
extern void krn_get_isr(uint8_t no, isr_st *dst);
/* kernel/mem.c */
extern void krn_mem_init(void);
/* kernel/speaker.c */
extern void krn_speaker_stop(void);
extern void krn_speaker_play(uint16_t hz);
/* kernel/timer.c */
extern void krn_timer_handle_intr(void);
extern uint32_t krn_timer_get_msecs(void);
extern void krn_timer_init(void);
extern void krn_timer_deinit(void);
/* kernel/vga.c */
extern const vga_theme_st krn_vga_themes[VGA_THEME_COUNT];
extern int krn_vga_current_theme;
extern void krn_vga_set_theme(int n);
extern void krn_vga_init(void);
extern void krn_vga_deinit(void);
