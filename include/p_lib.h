/* lib/bios.c */
extern void bios_putc(char c);
extern void bios_puts(const char *s);
extern uint16_t bios_getc(void);
extern uint16_t bios_get_key(void);
extern void bios_uart_init(void);
extern void bios_uart_putc(char c);
extern void bios_uart_puts(const char *s);
extern void bios_reboot(void);
/* lib/key.c */
extern char key_char_for_code(uint8_t code, uint8_t mods);
extern int key_number_for_code(uint8_t code);
/* lib/math.c */
extern int udiv32(uint32_t *out, uint32_t dividend, uint32_t divisor);
extern int umod32(uint32_t *out, uint32_t dividend, uint32_t divisor);
extern int add32(int32_t *out, int32_t a, int32_t b);
extern int sub32(int32_t *out, int32_t a, int32_t b);
extern int mul32(int32_t *out, int32_t a, int32_t b);
extern int div32(int32_t *out, int32_t a, int32_t b);
extern int append32(int32_t *out, int32_t a, int32_t d);
/* lib/printf.c */
extern int vsnprintf(char *buf, size_t nbyte, const char *fmt, va_list va);
extern int snprintf(char *buf, size_t nbyte, const char *fmt, ...);
/* lib/rand.c */
extern void rand_init(void);
extern uint16_t rand(void);
/* lib/sleep.c */
extern void sleep(uint32_t msecs);
extern void halt(void);
/* lib/string.c */
extern void * memcpy(void *dest, const void *src, size_t n);
extern void far * memcpy_far(void far *dest, const void far *src, size_t n);
extern void * memset(void *dest, int c, size_t n);
extern void far * memset_far(void far *dest, int c, size_t n);
extern int32_t strcmp(const char *s1, const char *s2);
extern size_t strlen(const char *s1);
extern char * strncpy(char *dest, const char *src, size_t n);
/* lib/tests.c */
extern void tests_run(void);
/* lib/time.c */
extern const char *TIME_MONTH_NAMES_SHORT[];
extern const char *TIME_DAY_NAMES_SHORT[];
extern const char *TIME_DAY_NAMES_LONG[];
extern int time_get_day_of_week(int day, int month, int year);
extern int time_get_days_in_month(int month, int year);
extern int time_equals(time_st *t1, time_st *t2);
extern void time_init(time_st *t, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
extern void time_clear(time_st *t);
extern void time_copy(time_st *dst, time_st *src);
extern void time_add_seconds(time_st *t, uint32_t secs);
extern void time_get(time_st *t);
extern void time_set(time_st *t, int set_rtc);
