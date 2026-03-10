#include <stdlib.h>

#include "ui.h"
#include "termbox2.h"

void ui_init() {
    if (tb_init() != TB_OK) {
        fprintf(stderr, "Failed to initialize termbox\n");
        exit(EXIT_FAILURE);
    }
}

void ui_shutdown() {
    tb_shutdown();
}

void ui_clear() {
    tb_clear();
}

void ui_present() {
    tb_present();
}

void ui_draw_header() {
    tb_printf(0, 0, 0, 0, "proctop");
    tb_printf(0, 1, 0, 0, "-----------------------");
}

void ui_draw_cpu(double cpu_usage){
    tb_printf(0, 2, 0, 0, "CPU: %.2f", cpu_usage);
}

void ui_draw_memory(double mem_used, double mem_total){
    tb_printf(0, 3, 0, 0, "MEM: %.2f / %.2f", mem_used, mem_total);
}

void ui_draw_process_table() {
    
}

int ui_should_quit() {
    tb_printf(0, 8, 0, 0, "Press Q to quit!");

    struct tb_event ev;
    tb_poll_event(&ev);

    if (ev.key == 'q' || ev.key == 'Q') 
        return 1;

    return 0;
}
