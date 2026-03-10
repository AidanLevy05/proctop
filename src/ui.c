#define TB_IMPL
#include "termbox2.h"
#include "ui.h"

static int ui_is_quit_event(const struct tb_event *event) {
    if (event->type != TB_EVENT_KEY) {
        return 0;
    }

    if (event->ch == 'q' || event->ch == 'Q') {
        return 1;
    }

    return event->key == TB_KEY_ESC;
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
}

ui_action ui_wait_for_action(void) {
    struct tb_event event;

    if (tb_poll_event(&event) <= 0) {
        return UI_ACTION_NONE;
    }

    if (ui_is_quit_event(&event)) {
        return UI_ACTION_QUIT;
    }

    return UI_ACTION_NONE;
}
