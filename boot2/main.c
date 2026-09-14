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
    KERNEL_START_LBA = 6,
    KERNEL_SIZE = 127,
    KERNEL_OFFSET = 0x100,

    INITRD_SEGMENT = 0x3000,
    INITRD_START_LBA = KERNEL_START_LBA + KERNEL_SIZE,
    INITRD_SIZE = 128,
    INITRD_OFFSET = 0,
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

typedef struct {
    uint8_t cylinder;
    uint8_t head;
    uint8_t sector;
} chs_st;

extern void intr(int, regs_st *);
extern void start_kernel(void);
extern void halt(void);
extern void stop_floppy_motor(void);

static uint8_t boot_drive_index = 0;
static uint8_t boot_drive_spt = 0;
static uint8_t boot_drive_heads = 0;

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

static void
load_drive_geometry(void)
{
    regs_st regs;

    regs_init(&regs);

    regs.h.ah = 0x08;
    regs.h.dl = boot_drive_index;

    /* Setting these 2 just in case, not sure if needed */
    regs.x.es = 0;
    regs.x.di = 0;

    intr(0x13, &regs);

    /* If the call is unsupported or returns garbage, use values for 720K/360K disks */
    if ((regs.x.flags & 0x0001) || (regs.h.cl & 0x3f) == 0) {
        boot_drive_spt = 9;
        boot_drive_heads = 2;
        return;
    }

    boot_drive_spt = regs.h.cl & 0x3f;
    boot_drive_heads = regs.h.dh + 1;
}

static void
lba_to_chs(uint16_t lba, chs_st *chs)
{
    uint16_t track = lba / boot_drive_spt;

    chs->sector = lba % boot_drive_spt + 1;
    chs->head = track % boot_drive_heads;
    chs->cylinder = track / boot_drive_heads;
}

static void
reset_drive(void)
{
    regs_st regs;

    regs_init(&regs);

    regs.h.ah = 0x00;
    regs.h.dl = boot_drive_index;

    intr(0x13, &regs);
}

static int
load_sectors(uint8_t n, const chs_st *chs, uint16_t seg, uint16_t ofs)
{
    regs_st regs;

    regs_init(&regs);

    regs.h.ah = 0x02;
    regs.h.al = n;
    regs.h.ch = chs->cylinder;
    regs.h.cl = chs->sector;
    regs.h.dh = chs->head;
    regs.h.dl = boot_drive_index;
    regs.x.bx = ofs;
    regs.x.es = seg;

    intr(0x13, &regs);

    return regs.x.flags & 0x0001;
}

static void
safe_load_sectors(uint8_t n, const chs_st *chs, uint16_t seg, uint16_t ofs)
{
    int i, status;

    for (i = 0; i < 3; ++i) {
        status = load_sectors(n, chs, seg, ofs);

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
load_region(uint16_t seg, uint16_t ofs, uint16_t lba, int count)
{
    chs_st chs;
    int remaining = count;
    unsigned n;

    while (remaining > 0) {
        lba_to_chs(lba, &chs);

        n = boot_drive_spt - (chs.sector - 1);

        if (n > remaining) {
            n = remaining;
        }

        safe_load_sectors(n, &chs, seg, ofs);

        remaining -= n;
        lba += n;
        ofs += n * 512;
    }
}

int
cmain(void)
{
    uint8_t *drive_ptr = 0;

    puts("\r\nLoading GentleOS [github.com/luke8086/gentleos]");

    boot_drive_index = *drive_ptr;
    reset_drive();

    load_drive_geometry();
    fix_diskette_param_table(boot_drive_spt);

    load_region(KERNEL_SEGMENT, KERNEL_OFFSET, KERNEL_START_LBA, KERNEL_SIZE);
    load_region(INITRD_SEGMENT, INITRD_OFFSET, INITRD_START_LBA, INITRD_SIZE);

    if (boot_drive_index == 0 || boot_drive_index == 1) {
        stop_floppy_motor();
    }

    start_kernel();

    halt();
    /* UNREACHABLE */
    return 0;
}
