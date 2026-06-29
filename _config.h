/*
 * Use custom color theme, see kernel/vga.c for the list
 * Requires a VGA card, it won't work on CGA
 */
#define DEFAULT_VGA_THEME 1

/*
 * Invert colors by default
 */
#define DEFAULT_COLORS_INVERTED 0

/*
 * Run test suite on startup
 */
#define ENABLE_TESTS 0

/*
 * Debug keyboard events (requires DEBUG_TO_UART)
 */
#define DEBUG_KEYBOARD 0

/*
 * Handle keyboard using BIOS calls instead of the native driver.
 * It's useful on custom devices without the 8042 keyboard controller,
 * like HP 200LX, but it doesn't support key-up events.
 */
#define USE_BIOS_KEYBOARD 0

/*
 * Send debug output to UART (COM1)
 */
#define DEBUG_TO_UART 0

/*
 * Default date & time to set on boot if the current year is below 2000.
 * This is useful for hardware with dead RTC battery
 */
#define DEFAULT_YEAR 2026
#define DEFAULT_MONTH 6
#define DEFAULT_DAY 1
#define DEFAULT_HOUR 12
#define DEFAULT_MINUTE 00
