#ifndef UI_H
#define UI_H

void ui_init();
void ui_shutdown();

void ui_clear();
void ui_present();

void ui_draw_header();

void ui_draw_cpu(double cpu_usage);
void ui_draw_memory(double mem_used, double mem_total);

void ui_draw_process_table();

int ui_should_quit();

#endif
