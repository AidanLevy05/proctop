/*
 * Uses termbox2 (MIT License)
 * https://github.com/termbox/termbox2
 */

#define TB_IMPL
#include "termbox2.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    struct tb_event ev;
    int y = 0;

    tb_init();

    tb_printf(0, y++, TB_GREEN, 0, "hello from termbox");
    tb_printf(0, y++, 0, 0, "width=%d height=%d", tb_width(), tb_height());
    tb_printf(0, y++, 0, 0, "press any key...");
    tb_present();

    tb_poll_event(&ev);
    
    y++;
    tb_printf(0, y++, 0, 0, "event type=%d key=%d ch=%c", ev.type, ev.key, ev.ch);
    tb_printf(0, y++, 0, 0, "press any key to quit...");
    tb_present();

    tb_poll_event(&ev);
    tb_shutdown();

    return 0;
}

