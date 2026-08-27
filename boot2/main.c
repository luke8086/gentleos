/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: main.c - Stage 2 bootloader C code
 */

/* Silence IDE warnings */
#ifdef __clang__
#define far
#endif

#ifdef __WATCOMC__
#define far __far
#endif

enum {
    STORAGE_SEGMENT = 0x0000,
    STORAGE_OFFSET_DPT = 0x0600,

    BOOT2_SEGMENT = 0x2000,

    KERNEL_SEGMENT = 0x1000,
    KERNEL_START_SECTOR = 7,
    KERNEL_SIZE = 127,
    KERNEL_OFFSET = 0x100,
};

typedef unsigned int uint16_t;
typedef unsigned char uint8_t;

typedef union {
    void far *ptr;

    struct {
        uint16_t ofs, seg;
    } w;
} farptr_st;

typedef union {
    struct {
        uint16_t ax, bx, cx, dx;
        uint16_t bp, di, si, flags;
        uint16_t ds, es;
    } x;

    struct {
        uint8_t al, ah;
        uint8_t bl, bh;
        uint8_t cl, ch;
        uint8_t dl, dh;
    } h;
} regs_st;

extern void intr(int, regs_st *);
extern void start_kernel(void);
extern void halt(void);

static uint8_t drive = 0;
static uint8_t sectors_per_track = 0;

/*
 * Copy BIOS's diskette parameter table to a new location and fix SPT value.
 * This is needed for some very old BIOSes, including one in MartyPC
 */
static void
fix_diskette_param_table(uint8_t spt)
{
    farptr_st src_fptr, dst_fptr, vec_fptr;
    uint16_t far *vec;
    uint8_t far *dst, far *src;
    int i;

    vec_fptr.w.seg = 0;
    vec_fptr.w.ofs = 0x1e * 4;
    vec = (uint16_t far *)vec_fptr.ptr;

    dst_fptr.w.seg = STORAGE_SEGMENT;
    dst_fptr.w.ofs = STORAGE_OFFSET_DPT;
    dst = (uint8_t far *)dst_fptr.ptr;

    src_fptr.w.seg = vec[1];
    src_fptr.w.ofs = vec[0];
    src = (uint8_t far *)src_fptr.ptr;

    for (i = 0; i < 11; ++i) {
        dst[i] = src[i];
    }

    dst[4] = spt;

    vec[0] = STORAGE_OFFSET_DPT;
    vec[1] = STORAGE_SEGMENT;
}

static void
regs_init(regs_st *regs)
{
    regs->x.ax = 0;
    regs->x.bx = 0;
    regs->x.cx = 0;
    regs->x.dx = 0;
    regs->x.bp = 0;
    regs->x.di = 0;
    regs->x.si = 0;
    regs->x.flags = 0;
    regs->x.ds = BOOT2_SEGMENT;
    regs->x.es = BOOT2_SEGMENT;
}

static void
putc(char c)
{
    regs_st regs;

    regs_init(&regs);

    regs.h.ah = 0x0e;
    regs.h.al = c;
    regs.x.bx = 0;

    intr(0x10, &regs);
}

static void
puts(const char *s)
{
    while (*s) {
        if ((*s) == '\n') {
            putc('\r');
        }

        putc(*s++);
    }
}

static uint8_t
get_sectors_per_track(void)
{
    regs_st regs;
    uint8_t spt;

    regs_init(&regs);

    regs.h.ah = 0x08;
    regs.h.dl = drive;

    /* Setting these 2 just in case, not sure if needed */
    regs.x.es = 0;
    regs.x.di = 0;

    intr(0x13, &regs);

    /* If the call is unsupported, return value for 720K/360K disks */
    if (regs.x.flags & 0x0001) {
        return 9;
    }

    spt = regs.h.cl & 0x3f;
    spt = spt ? spt : 9;

    return spt;
}

static void
reset_drive(void)
{
    regs_st regs;

    regs_init(&regs);

    regs.h.ah = 0x00;
    regs.h.dl = drive;

    intr(0x13, &regs);
}

static int
do_load_sectors(uint8_t n, uint8_t track, uint8_t head, uint8_t sector, unsigned target)
{
    regs_st regs;

    regs_init(&regs);

    regs.h.ah = 0x02;
    regs.h.al = n;
    regs.h.ch = track;
    regs.h.cl = sector;
    regs.h.dh = head;
    regs.h.dl = drive;
    regs.x.bx = target;
    regs.x.es = KERNEL_SEGMENT;

    intr(0x13, &regs);

    return regs.x.flags & 0x0001;
}

static void
load_sectors(uint8_t n, uint8_t track, uint8_t head, uint8_t sector, unsigned target)
{
    int i, status;

    for (i = 0; i < 3; ++i) {
        status = do_load_sectors(n, track, head, sector, target);

        if (status == 0) {
            putc('.');
            return;
        }

        reset_drive();
    }

    putc('E');
    halt();
    /* UNREACHABLE */
}

static void
load_kernel(void)
{
    int track = 0;
    int head = 0;
    int sector = KERNEL_START_SECTOR;
    int remaining = KERNEL_SIZE;
    unsigned target = KERNEL_OFFSET;
    unsigned n;

    while (remaining > 0) {
        n = sectors_per_track - (sector - 1);

        if (n > remaining) {
            n = remaining;
        }

        load_sectors(n, track, head, sector, target);

        remaining -= n;
        target += n * 512;
        sector = 1;
        head = head ? 0 : 1;

        if (head == 0) {
            ++track;
        }
    }
}

int
cmain(void)
{
    uint8_t *drive_ptr = 0;

    puts("\r\nLoading GentleOS [github.com/luke8086/gentleos]");

    drive = *drive_ptr;
    reset_drive();

    sectors_per_track = get_sectors_per_track();
    fix_diskette_param_table(sectors_per_track);

    load_kernel();
    start_kernel();

    halt();
    /* UNREACHABLE */
    return 0;
}
