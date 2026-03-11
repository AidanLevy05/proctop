#include "ui.h"
#include "proc.h"
#include "system.h"

#include <unistd.h>

int main(int argc, char **argv) {
    struct tb_event ev;
    int refresh = 1;

    if (argc > 1)
        proc_set_filter(argv[1]);

    ui_init();

    while (1) {
        if (refresh)
        {
            ui_clear();

            ui_draw_header();
            ui_draw_cpu(get_cpu_usage());
            ui_draw_memory(get_memory_used(), get_memory_total());
            ui_draw_process_table();
            ui_draw_status();

            ui_present();
            refresh = 0;
        }

        int rv = tb_peek_event(&ev, 1000);

        if (rv == TB_ERR_NO_EVENT) {
            refresh = 1;
            continue;
        }

        if (rv < 0) {
            refresh = 1;
            continue;
        }

        if (ev.type == TB_EVENT_KEY) {
            if (ev.ch == 'q' || ev.key == TB_KEY_ESC) {
                break;
            }
            if (ev.ch == 'r') {
                refresh = 1;
                continue;
            }
            if (ev.ch == 'm') {
                proc_set_sort_mode(PROC_SORT_MEM);
                refresh = 1;
                continue;
            }
            if (ev.ch == 'p') {
                proc_set_sort_mode(PROC_SORT_PID);
                refresh = 1;
                continue;
            }
            if (ev.ch == 'c') {
                proc_set_sort_mode(PROC_SORT_CPU);
                refresh = 1;
                continue;
            }
        }

        refresh = 1;
    }

    ui_shutdown();
    return 0;
}
