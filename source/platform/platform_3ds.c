#include "platform.h"

#include <3ds.h>
#include <stdio.h>

bool platform_init(void)
{
    osSetSpeedupEnable(true); /* New 3DS 804 MHz; no-op on Old 3DS */
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL); /* top screen belongs to citro3d */
    return true;
}

void platform_dispose(void)
{
    gfxExit();
}

bool platform_main_loop_running(void)
{
    return aptMainLoop();
}

uint32_t platform_time_ms(void)
{
    return (uint32_t)osGetTime();
}

void *platform_file_open(const char *path) { return fopen(path, "rb"); }

size_t platform_file_read(void *file, void *buffer, size_t size, uint64_t offset)
{
    if (fseek(file, (long)offset, SEEK_SET) != 0) return 0;
    return fread(buffer, 1, size, file);
}

void platform_file_close(void *file) { fclose(file); }

void platform_input_poll(platform_input *out)
{
    hidScanInput();
    circlePosition cp, cs;
    hidCircleRead(&cp);
    hidCstickRead(&cs);
    out->move_x = cp.dx; out->move_y = cp.dy;
    out->look_x = cs.dx; out->look_y = cs.dy;
    out->buttons = hidKeysHeld();
}
