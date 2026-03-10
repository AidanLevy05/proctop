#include "ui.h"

int main(void) {
    int running = 1;

    ui_init();

    while (running) {
        ui_clear();
        ui_draw_header();
        ui_draw_cpu(0.0);
        ui_draw_memory(0.0, 0.0);
        ui_draw_process_table();
        ui_draw_status();
        ui_present();

        running = ui_wait_for_action() != UI_ACTION_QUIT;
    }

    ui_shutdown();

    return 0;
}
