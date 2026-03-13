// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: planar.c - Support for VGA planar mode
// --------------------------------------------------------------------------------------

#include <gui.h>
#include "vga.h"

enum {
    FB_PITCH = GUI_WIDTH / 8,
    FB_PLANE_SIZE = GUI_HEIGHT * FB_PITCH,
};

#if GUI_PLANAR_MODE
static uint8_t gui_planar_pixels[4][FB_PLANE_SIZE] __attribute__((aligned(16)));
#else
static uint8_t **gui_planar_pixels;
#endif

void
gui_planar_flush(rect_st rect)
{
    int x0 = (rect.x / 8) * 8;
    int x1 = ((rect.x + rect.width + 7) / 8) * 8;
    int byte_x0 = x0 / 8;
    int byte_count = (x1 - x0) / 8;

    for (int plane = 0; plane < 4; ++plane) {
        gui_vga_set_write_planes(1 << plane);

        for (int y = rect.y; y < rect.y + rect.height; ++y) {
            memcpy(
                gui_fb_vram_surface->pixels + y * gui_fb_vram_surface->pitch + byte_x0,
                gui_planar_pixels[plane] + y * FB_PITCH + byte_x0,
                byte_count
            );
        }
    }
}

void
gui_planar_draw_rect(rect_st rect, uint8_t color)
{
    int l_x = rect.x;
    int r_x = rect.x + rect.width - 1;

    if (r_x < l_x) {
        return;
    }

    int l_byte = l_x / 8;
    int r_byte = r_x / 8;

    uint8_t l_mask = 0xFF >> (l_x & 7);
    uint8_t r_mask = 0xFF << (7 - (r_x & 7));

    for (int plane = 0; plane < 4; ++plane) {
        uint8_t fill = ((color >> plane) & 1) ? 0xFF : 0x00;
        uint8_t *dst_plane = gui_planar_pixels[plane];

        for (int y = rect.y; y < rect.y + rect.height; ++y) {
            uint8_t *dst_row = dst_plane + y * FB_PITCH;

            if (l_byte == r_byte) {
                uint8_t mask = l_mask & r_mask;
                dst_row[l_byte] = (dst_row[l_byte] & ~mask) | (fill & mask);
                continue;
            }

            dst_row[l_byte] = (dst_row[l_byte] & ~l_mask) | (fill & l_mask);

            if (r_byte > l_byte + 1) {
                memset(dst_row + l_byte + 1, fill, r_byte - l_byte - 1);
            }

            dst_row[r_byte] = (dst_row[r_byte] & ~r_mask) | (fill & r_mask);
        }
    }
}

void
gui_planar_draw_pattern(rect_st dst_rect, bitmap_st *pattern, uint8_t c1, uint8_t c2)
{
    int pat_w = pattern->size.width;
    int pat_h = pattern->size.height;

    uint8_t tile_pixels[pat_h * pat_w];

    surface_st tile = {
        .size = pattern->size,
        .pitch = pat_w,
        .pixels = tile_pixels,
    };

    for (int i = 0; i < pat_w * pat_h; ++i) {
        tile_pixels[i] = pattern->pixels[i] ? c1 : c2;
    }

    int ofs_x = dst_rect.x % pat_w;
    int ofs_y = dst_rect.y % pat_h;

    rect_st src_rect;

    for (int y = dst_rect.y; y < dst_rect.y + dst_rect.height; y += src_rect.height) {
        src_rect.y = (y - dst_rect.y + ofs_y) % pat_h;

        src_rect.height = pat_h - src_rect.y;
        if (src_rect.height + y > dst_rect.y + dst_rect.height) {
            src_rect.height = dst_rect.y + dst_rect.height - y;
        }

        for (int x = dst_rect.x; x < dst_rect.x + dst_rect.width; x += src_rect.width) {
            src_rect.x = (x - dst_rect.x + ofs_x) % pat_w;

            src_rect.width = pat_w - src_rect.x;
            if (x + src_rect.width > dst_rect.x + dst_rect.width) {
                src_rect.width = dst_rect.x + dst_rect.width - x;
            }

            gui_planar_draw_surface(x, y, &tile, src_rect);
        }
    }
}

void
gui_planar_draw_surface(int dst_x, int dst_y, surface_st *src, rect_st src_rect)
{
    if (src_rect.width <= 0 || src_rect.height <= 0) {
        return;
    }

#if GUI_PLANAR_MODE
    uint8_t (*dst)[FB_PLANE_SIZE] = (uint8_t (*)[FB_PLANE_SIZE])gui_planar_pixels;
#else
    uint8_t (*dst)[FB_PLANE_SIZE] = NULL;
#endif

    int dst_l_x = dst_x;
    int dst_r_x = dst_x + src_rect.width - 1;

    int dst_l_byte = dst_l_x / 8;
    int dst_r_byte = dst_r_x / 8;

    int dst_l_full_byte = (dst_l_x + 7) / 8;
    int dst_r_full_byte = (dst_r_x + 1) / 8;

    uint8_t dst_l_mask = 0xFF >> (dst_l_x & 7);
    uint8_t dst_r_mask = 0xFF << (7 - (dst_r_x & 7));

    for (int row = 0; row < src_rect.height; ++row) {
        uint8_t *src_row = src->pixels + (src_rect.y + row) * src->pitch + src_rect.x;
        int dst_row_ofs = (dst_y + row) * FB_PITCH;

        if (dst_l_byte == dst_r_byte) {
            uint8_t p[4] = { 0, 0, 0, 0 };
            uint8_t dst_mask = dst_l_mask & dst_r_mask;
            int dst_ofs = dst_row_ofs + dst_l_byte;

            for (int bit = 0; bit < 8; ++bit) {
                int src_x = dst_l_byte * 8 - dst_x + bit;

                if (src_x >= 0 && src_x < src_rect.width) {
                    uint8_t c = src_row[src_x];
                    uint8_t s = 7 - bit;

                    p[0] |= ((c     ) & 1) << s;
                    p[1] |= ((c >> 1) & 1) << s;
                    p[2] |= ((c >> 2) & 1) << s;
                    p[3] |= ((c >> 3) & 1) << s;
                }
            }

            for (int i = 0; i < 4; ++i) {
                dst[i][dst_ofs] = (dst[i][dst_ofs] & ~dst_mask) | p[i];
            }

            continue;
        }

        if (dst_l_byte < dst_l_full_byte) {
            uint8_t p[4] = { 0, 0, 0, 0 };
            int dst_ofs = dst_row_ofs + dst_l_byte;

            for (int bit = dst_l_x & 7; bit < 8; ++bit) {
                uint8_t c = src_row[dst_l_byte * 8 - dst_x + bit];
                uint8_t s = 7 - bit;

                p[0] |= ((c     ) & 1) << s;
                p[1] |= ((c >> 1) & 1) << s;
                p[2] |= ((c >> 2) & 1) << s;
                p[3] |= ((c >> 3) & 1) << s;
            }

            for (int i = 0; i < 4; ++i) {
                dst[i][dst_ofs] = (dst[i][dst_ofs] & ~dst_l_mask) | p[i];
            }
        }

        for (int x = dst_l_full_byte; x < dst_r_full_byte; ++x) {
            uint8_t p[4] = { 0, 0, 0, 0 };
            int dst_ofs = dst_row_ofs + x;

            for (int bit = 0; bit < 8; ++bit) {
                uint8_t c = src_row[x * 8 - dst_x + bit];
                uint8_t s = 7 - bit;

                p[0] |= ((c     ) & 1) << s;
                p[1] |= ((c >> 1) & 1) << s;
                p[2] |= ((c >> 2) & 1) << s;
                p[3] |= ((c >> 3) & 1) << s;
            }

            for (int i = 0; i < 4; ++i) {
                dst[i][dst_ofs] = p[i];
            }
        }

        if (dst_r_byte >= dst_r_full_byte) {
            uint8_t p[4] = { 0, 0, 0, 0 };
            int dst_ofs = dst_row_ofs + dst_r_byte;

            for (int bit = 0; bit <= (dst_r_x & 7); ++bit) {
                uint8_t c = src_row[dst_r_byte * 8 - dst_x + bit];
                uint8_t s = 7 - bit;

                p[0] |= ((c     ) & 1) << s;
                p[1] |= ((c >> 1) & 1) << s;
                p[2] |= ((c >> 2) & 1) << s;
                p[3] |= ((c >> 3) & 1) << s;
            }

            for (int i = 0; i < 4; ++i) {
                dst[i][dst_ofs] = (dst[i][dst_ofs] & ~dst_r_mask) | p[i];
            }
        }
    }
}

void
gui_planar_draw_pointer(int dst_x, int dst_y)
{
    bitmap_st *bitmap = &bitmap_pointer;
    uint8_t alpha = bitmap->alpha;

    int bmp_w = bitmap->size.width;
    int bmp_h = bitmap->size.height;

    if (dst_x + bmp_w > GUI_WIDTH) {
        bmp_w = GUI_WIDTH - dst_x;
    }

    if (dst_y + bmp_h > GUI_HEIGHT) {
        bmp_h = GUI_HEIGHT - dst_y;
    }

    if (bmp_w <= 0 || bmp_h <= 0) {
        return;
    }

    int dst_l_x = (dst_x / 8) * 8;
    int dst_r_x = ((dst_x + bmp_w + 7) / 8) * 8;

    int dst_l_byte = dst_l_x / 8;
    int dst_byte_count = (dst_r_x - dst_l_x) / 8;

    for (int plane = 0; plane < 4; ++plane) {
        gui_vga_set_write_planes(1 << plane);

        for (int row = 0; row < bmp_h; ++row) {
            int y = dst_y + row;

            uint8_t *dst = gui_fb_vram_surface->pixels + y * FB_PITCH + dst_l_byte;

            for (int byte = 0; byte < dst_byte_count; ++byte) {
                uint8_t val = 0;

                for (int bit = 0; bit < 8; ++bit) {
                    int px_x = dst_l_x + byte * 8 + bit;
                    int bmp_x = px_x - dst_x;

                    if (bmp_x >= 0 && bmp_x < bmp_w &&
                        bitmap->pixels[row * bitmap->size.width + bmp_x] != alpha) {

                        uint8_t c = bitmap->pixels[row * bitmap->size.width + bmp_x];
                        val |= ((c >> plane) & 1) << (7 - bit);
                    } else {
                        int byte_ofs = y * FB_PITCH + px_x / 8;
                        int bit_pos = 7 - (px_x & 7);
                        uint8_t c = (gui_planar_pixels[plane][byte_ofs] >> bit_pos) & 1;
                        val |= c << (7 - bit);
                    }
                }

                dst[byte] = val;
            }
        }
    }
}

// This must be only be called from gui_planar_xor_corners()
static void
gui_planar_xor_h_seg(uint8_t *vram, int x, int y, int w)
{
    if (w <= 0) {
        return;
    }

    int l_x = x;
    int r_x = x + w - 1;
    int l_byte = l_x / 8;
    int r_byte = r_x / 8;

    uint8_t l_mask = 0xFF >> (l_x & 7);
    uint8_t r_mask = 0xFF << (7 - (r_x & 7));

    uint8_t *row = vram + y * FB_PITCH;

    if (l_byte == r_byte) {
        gui_vga_set_bit_mask(l_mask & r_mask);
        gui_vga_latch_write(&row[l_byte], 0xFF);
        return;
    }

    gui_vga_set_bit_mask(l_mask);
    gui_vga_latch_write(&row[l_byte], 0xFF);

    if (r_byte > l_byte + 1) {
        gui_vga_set_bit_mask(0xFF);
        for (int b = l_byte + 1; b < r_byte; ++b) {
            gui_vga_latch_write(&row[b], 0xFF);
        }
    }

    gui_vga_set_bit_mask(r_mask);
    gui_vga_latch_write(&row[r_byte], 0xFF);
}

// This must be only be called from gui_planar_xor_corners()
static void
gui_planar_xor_v_seg(uint8_t *vram, int x, int y, int h)
{
    if (h <= 0) {
        return;
    }

    int x_byte = x / 8;
    uint8_t mask = 0x80 >> (x & 7);

    gui_vga_set_bit_mask(mask);

    for (int row = 0; row < h; ++row) {
        gui_vga_latch_write(vram + (y + row) * FB_PITCH + x_byte, 0xFF);
    }
}

void
gui_planar_xor_corners(rect_st rect)
{
    uint8_t *vram = gui_fb_vram_surface->pixels;
    const int arm = 16;

    int l_x = rect.x;
    int t_y = rect.y;
    int r_x = rect.x + rect.width - 1;
    int b_y = rect.y + rect.height - 1;

    gui_vga_set_write_planes(0x0F);
    gui_vga_set_logic_op(0x18);

    // Top-left
    gui_planar_xor_h_seg(vram, l_x, t_y, arm);
    gui_planar_xor_v_seg(vram, l_x, t_y + 1, arm - 1);

    // Top-right
    gui_planar_xor_h_seg(vram, r_x - arm + 1, t_y, arm);
    gui_planar_xor_v_seg(vram, r_x, t_y + 1, arm - 1);

    // Bottom-left
    gui_planar_xor_h_seg(vram, l_x, b_y, arm);
    gui_planar_xor_v_seg(vram, l_x, b_y - arm + 1, arm - 1);

    // Bottom-right
    gui_planar_xor_h_seg(vram, r_x - arm + 1, b_y, arm);
    gui_planar_xor_v_seg(vram, r_x, b_y - arm + 1, arm - 1);

    gui_vga_set_logic_op(0x00);
    gui_vga_set_bit_mask(0xFF);
}
