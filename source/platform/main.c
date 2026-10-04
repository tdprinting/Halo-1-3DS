#include "platform.h"

#include <3ds.h>
#include <stdio.h>

int main(void)
{
    if (!platform_init()) return 1;

    printf("Halo 1 (3DS port, bring-up build)\n");
    printf("Game logic not linked yet. See docs/PORTING_PLAN.md\n");
    printf("START to exit\n");

    while (platform_main_loop_running()) {
        platform_input input;
        platform_input_poll(&input);
        if (input.buttons & KEY_START) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    platform_dispose();
    return 0;
}
