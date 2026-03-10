#include "ui.h"
#include "system.h"

#include <unistd.h>

int main(void) {
    struct tb_event ev;

    ui_init();

    while (1) {

        ui_clear();

        ui_draw_header();
        ui_draw_cpu(get_cpu_usage());
        ui_draw_memory(0.0, 0.0);
        ui_draw_process_table();
        ui_draw_status();

        ui_present();

        tb_poll_event(&ev);

        if (ev.type == TB_EVENT_KEY) {
            if (ev.ch == 'q' || ev.key == TB_KEY_ESC) {
                break;
            }
        }
        usleep(100000);
    }

    ui_shutdown();
    return 0;
}
