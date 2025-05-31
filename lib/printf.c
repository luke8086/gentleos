// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: printf.c - Basic implementation of snprintf
// --------------------------------------------------------------------------------------

#include <lib.h>

#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__ size_t;
#else
typedef unsigned long size_t;
#endif

enum {
    PF_STATE_DEFAULT,
    PF_STATE_FLAGS,
    PF_STATE_WIDTH,
    PF_STATE_LENGTH,
    PF_STATE_CONV,
};

enum {
    PF_LEN_NONE,
    PF_LEN_H,
    PF_LEN_L,
};

enum {
    PF_CONV_c,
    PF_CONV_s,
    PF_CONV_d,
    PF_CONV_u,
    PF_CONV_x,
    PF_CONV_X,
};

typedef int (pf_emit_fn)(void *payload, char c);
typedef unsigned long (pf_arg_fn)(void *payload);

struct pf_config {
    pf_emit_fn *emit_fn;
    void *emit_payload;

    pf_arg_fn *arg_fn;
    void *arg_payload;
    va_list *arg_list;

    int emitted;
    int error;
    int state;

    int zpad;
    int rpad;
    int width;
    int length;
    int conv;
};

struct pf_vasnprintf_payload {
    char *buf;
    size_t i;
    size_t nbyte;
};

static unsigned long
pf_get_arg_va(va_list *va, int len, int conv)
{
    if (conv == PF_CONV_x || conv == PF_CONV_X)
        conv = PF_CONV_u;

    if (conv == PF_CONV_c && len == PF_LEN_NONE) {
        return va_arg(*va, int);
    }

    if (conv == PF_CONV_s && len == PF_LEN_NONE)
        return (unsigned long)va_arg(*va, char *);

    if (conv == PF_CONV_d && len == PF_LEN_H)
        return va_arg(*va, int);

    if (conv == PF_CONV_d && len == PF_LEN_NONE)
        return va_arg(*va, int);

    if (conv == PF_CONV_d && len == PF_LEN_L)
        return va_arg(*va, long);

    if (conv == PF_CONV_u && len == PF_LEN_H)
        return va_arg(*va, unsigned);

    if (conv == PF_CONV_u && len == PF_LEN_NONE)
        return va_arg(*va, unsigned);

    if (conv == PF_CONV_u && len == PF_LEN_L)
        return va_arg(*va, unsigned long);

    return 0;
}

static unsigned long
pf_get_arg(struct pf_config *c)
{
    if (c->arg_list)
        return pf_get_arg_va(c->arg_list, c->length, c->conv);

    if (c->arg_fn)
        return c->arg_fn(c->arg_payload);

    return 0;
}

static void
pf_emit(struct pf_config *c, char ch)
{
    if (c->error)
        return;

    if (c->emit_fn(c->emit_payload, ch)) {
        c->error = 1;
    } else {
        c->emitted++;
    }
}

static void
pf_emit_char(struct pf_config *c, char ch)
{
    int pad;

    // Emit left padding
    for (pad = c->width - 1; !c->rpad && pad > 0; --pad) {
        pf_emit(c, ' ');
    }

    // Emit character
    pf_emit(c, ch);


    // Emit right padding
    for (pad = c->width - 1; c->rpad && pad > 0; --pad) {
        pf_emit(c, ' ');
    }
}

static void
pf_emit_str(struct pf_config *c, char *s)
{
    int pad, len;

    len = strlen(s);

    // Emit left padding
    for (pad = c->width - len; !c->rpad && pad > 0; --pad) {
        pf_emit(c, ' ');
    }

    // Emit string
    while (*s) {
        pf_emit(c, *s++);
    }

    // Emit right padding
    for (pad = c->width - len; c->rpad && pad > 0; --pad) {
        pf_emit(c, ' ');
    }

}

static void
pf_emit_uint(struct pf_config *c, unsigned long n, int neg)
{
    static const char *l_hex_digits = "0123456789abcdef";
    static const char *u_hex_digits = "0123456789ABCDEF";

    char buf[32];
    const char *digits;
    int i, d, base;
    int pad;
    int num_width, sign_width;

    sign_width = !!neg;

    // Crop the value to the requested size
    switch (c->length) {
    case PF_LEN_H:      n = (unsigned short)n;      break;
    case PF_LEN_NONE:   n = (unsigned)n;            break;
    case PF_LEN_L:      n = (unsigned long)n;       break;
    };

    // Select uppercase or lowercase character set
    switch (c->conv) {
    case PF_CONV_d: base = 10; digits = l_hex_digits; break;
    case PF_CONV_u: base = 10; digits = l_hex_digits; break;
    case PF_CONV_x: base = 16; digits = l_hex_digits; break;
    case PF_CONV_X: base = 16; digits = u_hex_digits; break;
    default: c->error = 1; return;
    }

    // Save digits to the temporary buffer in a reverse order
    i = 0;
    do {
        d = n % base;
        n = n / base;
        buf[i++] = digits[d];
    } while (n != 0 && (size_t)i < sizeof(buf));

    // Save the amount of digits
    num_width = i;

    // In case of zero padding of a negative number, emit '-' before padding
    if (neg && c->zpad) {
        pf_emit(c, '-');
    }

    // Emit left padding (spaces or zeros)
    for (pad = c->width - num_width - sign_width; !c->rpad && pad > 0; --pad) {
        pf_emit(c, c->zpad ? '0' : ' ');
    }

    // In case of space padding of a negative number, emit '-' after padding
    if (neg && !c->zpad) {
        pf_emit(c, '-');
    }

    // Emit digits in the correct order
    for (i = num_width - 1; i >= 0; --i) {
        pf_emit(c, buf[i]);
    }

    // Emit right padding (only spaces)
    for (pad = c->width - num_width - sign_width; c->rpad && pad > 0; --pad) {
        pf_emit(c, ' ');
    }
}

static void
pf_emit_int(struct pf_config *c, long n)
{
    int neg = 0;

    if (n < 0) {
        neg = 1;
        n *= -1;
    }

    pf_emit_uint(c, (unsigned long)n, neg);
}

static void
pf_cprintf(const char *fmt, struct pf_config *c)
{
    unsigned long arg;
    char ch;

    c->state = PF_STATE_DEFAULT;
    c->error = 0;
    c->emitted = 0;

    while ((ch = *fmt++)) {

        //
        // PF_STATE_DEFAULT
        //

        // Regular character
        if (c->state == PF_STATE_DEFAULT && ch != '%') {
            pf_emit(c, ch);
            continue;
        }

        // Beginning of format specifier
        if (c->state == PF_STATE_DEFAULT && ch == '%') {
            c->state = PF_STATE_FLAGS;
            c->length = PF_LEN_NONE;
            c->conv = PF_LEN_NONE;
            c->width = 0;
            c->zpad = 0;
            c->rpad = 0;
            continue;
        }

        //
        // PF_STATE_FLAGS
        //

        // Handle flag '0'
        if (c->state == PF_STATE_FLAGS && ch == '0') {
            c->zpad = 1;
            continue;
        }

        // Handle flag '-'
        if (c->state == PF_STATE_FLAGS && ch == '-') {
            c->rpad = 1;
            continue;
        }

        // Handle character other than a flag
        if (c->state == PF_STATE_FLAGS) {
            c->state = PF_STATE_WIDTH;
            // FALLTHROUGH
        }

        //
        // PF_STATE_WIDTH
        //

        // Handle a single digit of the width
        if (c->state == PF_STATE_WIDTH && ch >= '0' && ch <= '9') {
            c->width = c->width * 10 + (ch - '0');
            continue;
        }

        // Handle character other than a digit
        if (c->state == PF_STATE_WIDTH) {
            c->state = PF_STATE_LENGTH;
            // FALLTHROUGH
        }

        //
        // PF_STATE_LENGTH
        //

        // Handle %h
        if (c->state == PF_STATE_LENGTH && c->length == PF_LEN_NONE && ch == 'h') {
            c->length = PF_LEN_H;
            continue;
        }

        // Handle %l
        if (c->state == PF_STATE_LENGTH && c->length == PF_LEN_NONE && ch == 'l') {
            c->length = PF_LEN_L;
            continue;
        }

        // Handle character other than a length specifier
        if (c->state == PF_STATE_LENGTH) {
            c->state = PF_STATE_CONV;
            // FALLTHROUGH
        }

        //
        // PF_STATE_CONV
        //

        // Handle %d
        if (c->state == PF_STATE_CONV && ch == 'd') {
            c->conv = PF_CONV_d;
            arg = pf_get_arg(c);
            pf_emit_int(c, (long)arg);
            c->state = PF_STATE_DEFAULT;
            continue;
        }

        // Handle %u
        if (c->state == PF_STATE_CONV && ch == 'u') {
            c->conv = PF_CONV_u;
            arg = pf_get_arg(c);
            pf_emit_uint(c, (unsigned long)arg, 0);
            c->state = PF_STATE_DEFAULT;
            continue;
        }

        // Handle %x
        if (c->state == PF_STATE_CONV && ch == 'x') {
            c->conv = PF_CONV_x;
            arg = pf_get_arg(c);
            pf_emit_uint(c, (unsigned long)arg, 0);
            c->state = PF_STATE_DEFAULT;
            continue;
        }

        // Handle %X
        if (c->state == PF_STATE_CONV && ch == 'X') {
            c->conv = PF_CONV_X;
            arg = pf_get_arg(c);
            pf_emit_uint(c, (unsigned long)arg, 0);
            c->state = PF_STATE_DEFAULT;
            continue;
        }

        // Handle %c
        if (c->state == PF_STATE_CONV && c->length == PF_LEN_NONE && ch == 'c') {
            c->conv = PF_CONV_c;
            arg = pf_get_arg(c);
            pf_emit_char(c, (char)arg);
            c->state = PF_STATE_DEFAULT;
            continue;
        }

        // Handle %s
        if (c->state == PF_STATE_CONV && c->length == PF_LEN_NONE && ch == 's') {
            c->conv = PF_CONV_s;
            arg = pf_get_arg(c);
            pf_emit_str(c, (char *)arg);
            c->state = PF_STATE_DEFAULT;
            continue;
        }

        // Invalid format
        c->error = 1;
    }

    pf_emit(c, 0);
}

static int
pf_vasnprintf_emit(void *payload, char ch)
{
    struct pf_vasnprintf_payload *p = (struct pf_vasnprintf_payload *)payload;

    if (p->i < p->nbyte)
        p->buf[p->i] = ch;

    p->i++;

    return 0;
}

static int
pf_vasnprintf(char *buf, size_t nbyte, const char *fmt,
              va_list *arg_list, pf_arg_fn *arg_fn, void *arg_payload)
{
    struct pf_vasnprintf_payload payload, *p = &payload;
    struct pf_config config, *c = &config;

    // Setup payload
    p->buf = buf;
    p->nbyte = nbyte;
    p->i = 0;

    // Setup pf_config
    c->emit_fn = pf_vasnprintf_emit;
    c->emit_payload = p;
    c->arg_list = arg_list;
    c->arg_fn = arg_fn;
    c->arg_payload = arg_payload;

    // Process
    pf_cprintf(fmt, c);

    // In case of overflow, ensure that buffer is null-terminated
    if (p->i > p->nbyte) {
        p->buf[p->nbyte - 1] = 0;
    }

    // The return value is the amount of characters which would
    // be emitted, given enough space, or -1 on error
    return c->error ? -1 : (p->i - 1);
}

int
vsnprintf(char *buf, size_t nbyte, const char *fmt, va_list va)
{
    va_list va_copy;
    int ret;

    va_copy(va_copy, va);
    ret = pf_vasnprintf(buf, nbyte, fmt, &va_copy, 0, 0);
    va_end(va_copy);

    return ret;
}

int
snprintf(char *buf, size_t nbyte, const char *fmt, ...)
{
    va_list va;
    int ret;

    va_start(va, fmt);
    ret = pf_vasnprintf(buf, nbyte, fmt, &va, 0, 0);
    va_end(va);

    return ret;
}
