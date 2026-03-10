/*
 * Uses termbox2 (MIT License)
 * https://github.com/termbox/termbox2
 */

#define TB_IMPL
#include "termbox2.h"

#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

    struct tb_event ev;

    ui_init();

    while (!ui_should_quit()) {
        
        ui_clear();

        ui_draw_header();
        ui_draw_cpu(0.0);
        ui_draw_memory(0.0, 0.0);
        ui_draw_process_table();

        ui_present();


    }

    ui_shutdown();

    return 0;
}

