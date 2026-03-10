#define TB_IMPL
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int ui_should_quit(struct tb_event *event) {
    tb_poll_event(event);

    if (event->type == TB_EVENT_KEY) {
        if (event->ch == 'q' || event->key == TB_KEY_ESC) {
            return 1;
        }
    }

    return 0;
}

void ui_init(void) {
    if (tb_init_rwfd(STDIN_FILENO, STDOUT_FILENO) != TB_OK) {
        fprintf(stderr, "Failed to initialize termbox\n");
        exit(EXIT_FAILURE);
    }

    tb_set_input_mode(TB_INPUT_ESC);
}

void ui_shutdown(void) {
    tb_shutdown();
}

void ui_clear(void) {
    tb_clear();
}

void ui_present(void) {
    tb_present();
}

void ui_draw_header(void) {
    tb_printf(0, 0, 0, 0, "proctop");
    tb_printf(0, 1, 0, 0, "-----------------------");
}

void ui_draw_status(void) {
    tb_printf(0, 8, 0, 0, "Press Q or Esc to quit");
}

void ui_draw_cpu(double cpu_usage) {
    tb_printf(0, 2, 0, 0, "CPU: %.2f", cpu_usage);
}

void ui_draw_memory(double mem_used, double mem_total) {
    tb_printf(0, 3, 0, 0, "MEM: %.2f / %.2f", mem_used, mem_total);
}

void ui_draw_process_table(void) {
    tb_printf(0, 5, 0, 0, "PID     USER     CPU%%    MEM%%    COMMAND");
    tb_printf(0, 6, 0, 0, "------------------------------------------");
    tb_printf(0, 7, 0, 0, "1234    aidan    0.0     0.0     proctop");
}
