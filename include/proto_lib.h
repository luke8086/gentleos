/* lib/printf.c */
extern int vsnprintf(char *buf, size_t nbyte, const char *fmt, va_list va);
extern int snprintf(char *buf, size_t nbyte, const char *fmt, ...);
/* lib/rand.c */
extern void rand_init(void);
extern uint32_t rand(void);
/* lib/string.c */
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *dest, int c, size_t n);
extern int32_t strcmp(const char *s1, const char *s2);
extern size_t strlen(const char *s1);
extern char *strncpy(char *dest, const char *src, size_t n);
