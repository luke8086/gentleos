#define GUI_WIDTH 800
#define GUI_HEIGHT 600

// Workaround for PCs where GRUB only manages to enable
// 4-bit planar mode and still reports it as 1-bit mode
// Setting this to 1 enforces 4-bit planar mode
// It may also require adding insmod all_video in grub.cfg
#define GUI_PLANAR_MODE 0
