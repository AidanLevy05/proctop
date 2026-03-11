#ifndef UI_H
#define UI_H

#include "termbox2.h"

int ui_should_quit(struct tb_event *event);
void ui_init(void);
void ui_shutdown(void);
void ui_clear(void);
void ui_present(void);
void ui_draw_header(void);
void ui_draw_system(double cpu_usage, double mem_used, double mem_total,
                    double load1, double load5, double load15, double uptime);
void ui_draw_gpu(double *gpu_usage, double *gpu_mem_used, double *gpu_mem_total, int count);
void ui_draw_core_cpu(double *cpu_usage, int count);
void ui_draw_status(void);
void ui_draw_cpu(double cpu_usage);
void ui_draw_memory(double mem_used, double mem_total);
void ui_draw_process_table(void);
void ui_draw_process_details(void);
void ui_move_selection(int delta);
int ui_get_selected_pid(void);
void ui_set_status_message(const char *message);
void ui_clear_status_message(void);
void ui_log_processes(FILE *f);

#endif
