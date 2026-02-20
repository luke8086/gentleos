/* kernel/core_c.c */
extern uint32_t krn_core_mboot_header[];
extern mboot_info_st *krn_core_mboot_info;
extern void krn_core_c_main(void);
extern void krn_core_c_isr_handle(isr_stack_st *isr_stack);
/* kernel/debug.c */
extern void krn_debug_printf(const char *fmt, ...);
extern void krn_debug_dump_multiboot_info(void);
extern void krn_debug_dump_kernel_location(void);
/* kernel/event.c */
extern int krn_event_ipush(event_st event);
extern int krn_event_push(event_st event);
extern int krn_event_pop(event_st *event);
extern uint16_t krn_event_count(void);
/* kernel/interrupt.c */
extern void krn_interrupt_handle(isr_stack_st *isr_stack);
extern void krn_interrupt_set_handler(uint8_t int_no, isr_handler_fn handler);
/* kernel/keyboard.c */
extern void krn_keyboard_init(void);
/* kernel/main.c */
extern void krn_main(void);
/* kernel/mouse.c */
extern void krn_mouse_init(void);
/* kernel/rtc.c */
extern int krn_rtc_are_times_equal(time_st *t1, time_st *t2);
extern void krn_rtc_get_time(time_st *t);
/* kernel/speaker.c */
extern void krn_speaker_stop(void);
extern void krn_speaker_play(unsigned hz);
/* kernel/timer.c */
extern void krn_timer_init(void);
