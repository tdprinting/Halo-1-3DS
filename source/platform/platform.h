/* Platform abstraction layer: everything the Xbox build gets from Xbox/D3D8/DSound/Bink
 * goes through here on 3DS. Keep this small; the game code should not see libctru. */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* lifecycle */
bool platform_init(void);
void platform_dispose(void);
bool platform_main_loop_running(void);

/* time: milliseconds since boot (replaces Xbox KeQueryPerformanceCounter use) */
uint32_t platform_time_ms(void);

/* storage: map data is read from the SD card (sdmc:/halo/) */
void *platform_file_open(const char *path);
size_t platform_file_read(void *file, void *buffer, size_t size, uint64_t offset);
void platform_file_close(void *file);

/* input: polled once per frame */
typedef struct {
    int16_t move_x, move_y;   /* circle pad / New 3DS: left stick */
    int16_t look_x, look_y;   /* C-stick */
    uint32_t buttons;         /* bit i = PLATFORM_BUTTON_* */
} platform_input;
void platform_input_poll(platform_input *out);

#endif
