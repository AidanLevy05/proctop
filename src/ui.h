#ifndef UI_H
#define UI_H

typedef enum {
    UI_ACTION_NONE = 0,
    UI_ACTION_QUIT
} ui_action;

void ui_init(void);
void ui_shutdown(void);

void ui_clear(void);
void ui_present(void);

void ui_draw_header(void);
void ui_draw_status(void);

void ui_draw_cpu(double cpu_usage);
void ui_draw_memory(double mem_used, double mem_total);

void ui_draw_process_table(void);

ui_action ui_wait_for_action(void);

#endif
