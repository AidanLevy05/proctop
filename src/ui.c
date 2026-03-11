#define TB_IMPL
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proc.h"

static struct process current_procs[MAX_PROCS];
static int current_proc_count = 0;
static int current_visible = 0;
static int current_selected = 0;
static int header_gpu_lines = 0;
static int header_core_lines = 0;
static double current_mem_total = 0.0;

static int ui_draw_text(int x, int y, uintattr_t fg, uintattr_t bg, const char *text) {
    tb_printf(x, y, fg, bg, "%s", text);
    return strlen(text);
}

static uintattr_t ui_usage_color(double usage) {
    if (usage >= 75.0)
        return TB_RED | TB_BOLD;
    if (usage >= 40.0)
        return TB_YELLOW | TB_BOLD;
    return TB_GREEN | TB_BOLD;
}

static uintattr_t ui_memory_color(double mem_mb) {
    double total_mb;
    double ratio;

    if (current_mem_total <= 0.0)
        return TB_DEFAULT;

    total_mb = current_mem_total * 1024.0;
    ratio = mem_mb / total_mb;

    if (ratio >= 0.15)
        return TB_RED | TB_BOLD;
    if (ratio >= 0.05)
        return TB_YELLOW | TB_BOLD;
    return TB_GREEN | TB_BOLD;
}

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
    tb_printf(0, 0, TB_CYAN | TB_BOLD, TB_DEFAULT, "proctop");
    tb_printf(0, 1, TB_CYAN, TB_DEFAULT, "-----------------------");
}

void ui_draw_system(double cpu_usage, double mem_used, double mem_total,
                    double load1, double load5, double load15) {
    char buf[64];
    int x = 0;

    current_mem_total = mem_total;

    x += ui_draw_text(x, 2, TB_CYAN | TB_BOLD, TB_DEFAULT, "CPU: ");
    snprintf(buf, sizeof(buf), "%.2f%%", cpu_usage);
    x += ui_draw_text(x, 2, ui_usage_color(cpu_usage), TB_DEFAULT, buf);

    x += ui_draw_text(x, 2, TB_DEFAULT, TB_DEFAULT, "  ");
    x += ui_draw_text(x, 2, TB_MAGENTA | TB_BOLD, TB_DEFAULT, "MEM: ");
    snprintf(buf, sizeof(buf), "%.2f / %.2f GB", mem_used, mem_total);
    x += ui_draw_text(x, 2, ui_memory_color(mem_used * 1024.0), TB_DEFAULT, buf);

    x += ui_draw_text(x, 2, TB_DEFAULT, TB_DEFAULT, "  ");
    x += ui_draw_text(x, 2, TB_YELLOW | TB_BOLD, TB_DEFAULT, "LOAD: ");
    snprintf(buf, sizeof(buf), "%.2f %.2f %.2f", load1, load5, load15);
    ui_draw_text(x, 2, TB_YELLOW, TB_DEFAULT, buf);
}

void ui_draw_gpu(double *gpu_usage, double *gpu_mem_used, double *gpu_mem_total, int count) {
    header_gpu_lines = 0;

    if (count <= 0) {
        ui_draw_text(0, 3, TB_MAGENTA | TB_BOLD, TB_DEFAULT, "GPU: ");
        ui_draw_text(5, 3, TB_YELLOW, TB_DEFAULT, "N/A");
        header_gpu_lines = 1;
        return;
    }

    for (int i = 0; i < count; i++) {
        char buf[64];
        int x = 0;
        int y = 3 + i;

        snprintf(buf, sizeof(buf), "GPU%d: ", i);
        x += ui_draw_text(x, y, TB_MAGENTA | TB_BOLD, TB_DEFAULT, buf);

        snprintf(buf, sizeof(buf), "%.0f%%", gpu_usage[i]);
        x += ui_draw_text(x, y, ui_usage_color(gpu_usage[i]), TB_DEFAULT, buf);

        x += ui_draw_text(x, y, TB_DEFAULT, TB_DEFAULT, "  ");
        x += ui_draw_text(x, y, TB_MAGENTA | TB_BOLD, TB_DEFAULT, "VRAM: ");

        snprintf(buf, sizeof(buf), "%.1f / %.1f GB", gpu_mem_used[i], gpu_mem_total[i]);
        if (gpu_mem_total[i] > 0.0)
            ui_draw_text(x, y, ui_memory_color((gpu_mem_used[i] / gpu_mem_total[i]) * current_mem_total * 1024.0), TB_DEFAULT, buf);
        else
            ui_draw_text(x, y, TB_DEFAULT, TB_DEFAULT, buf);
    }

    header_gpu_lines = count;
}

void ui_draw_core_cpu(double *cpu_usage, int count) {
    int cols = tb_width() / 14;

    if (cols < 1)
        cols = 1;

    header_core_lines = 0;

    for (int i = 0; i < count; i++) {
        char label[16];
        char value[16];
        int x = (i % cols) * 14;
        int y = 3 + header_gpu_lines + (i / cols);

        snprintf(label, sizeof(label), "cpu%d:", i);
        snprintf(value, sizeof(value), "%5.1f%%", cpu_usage[i]);

        ui_draw_text(x, y, TB_CYAN, TB_DEFAULT, label);
        ui_draw_text(x + 5, y, ui_usage_color(cpu_usage[i]), TB_DEFAULT, value);
    }

    if (count > 0)
        header_core_lines = (count + cols - 1) / cols;
}

void ui_draw_status(void) {
    int h = tb_height();
    int x = 0;

    if (h <= 0)
        return;

    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "q");
    x += ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " quit  ");
    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "r");
    x += ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " refresh  ");
    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "m");
    x += ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " mem  ");
    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "p");
    x += ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " pid  ");
    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "c");
    x += ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " cpu  ");
    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "up/down");
    x += ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " move  ");
    x += ui_draw_text(x, h - 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "esc");
    ui_draw_text(x, h - 1, TB_YELLOW, TB_DEFAULT, " quit");
}

void ui_draw_cpu(double cpu_usage) {
    tb_printf(0, 2, 0, 0, "CPU: %.2f%%", cpu_usage);
}

void ui_draw_memory(double mem_used, double mem_total) {
    tb_printf(0, 3, 0, 0, "MEM: %.2f / %.2f GB", mem_used, mem_total);
}

void ui_move_selection(int delta) {
    if (current_visible <= 0) {
        current_selected = 0;
        return;
    }

    current_selected += delta;

    if (current_selected < 0)
        current_selected = 0;
    if (current_selected >= current_visible)
        current_selected = current_visible - 1;
}

void ui_draw_process_table(void) {
    int y = 4 + header_gpu_lines + header_core_lines;
    int h = tb_height();
    int limit = h - y - 6;

    current_proc_count = proc_get_list(current_procs);

    if (limit < 0)
        limit = 0;
    if (limit > PROC_DISPLAY_LIMIT)
        limit = PROC_DISPLAY_LIMIT;
    if (limit > current_proc_count)
        limit = current_proc_count;

    current_visible = limit;

    if (current_visible <= 0)
        current_selected = 0;
    else if (current_selected >= current_visible)
        current_selected = current_visible - 1;

    tb_printf(0, y, TB_CYAN | TB_BOLD, TB_DEFAULT, "PID    USER        CPU%%   MEM(MB)   COMMAND");
    tb_printf(0, y + 1, TB_CYAN, TB_DEFAULT, "------------------------------------------------");

    for (int i = 0; i < current_visible; i++) {
        uintattr_t fg = TB_DEFAULT;
        uintattr_t bg = TB_DEFAULT;
        char buf[128];
        int row_y = y + 2 + i;

        if (i == current_selected) {
            tb_printf(0, row_y, TB_BLACK | TB_BOLD, TB_CYAN,
                    "%-6d %-10s %5.1f%% %8.1f MB  %s",
                    current_procs[i].pid,
                    current_procs[i].user,
                    current_procs[i].cpu_percent,
                    current_procs[i].mem_mb,
                    current_procs[i].command);
            continue;
        }

        snprintf(buf, sizeof(buf), "%-6d", current_procs[i].pid);
        ui_draw_text(0, row_y, fg, bg, buf);

        snprintf(buf, sizeof(buf), "%-10s", current_procs[i].user);
        ui_draw_text(7, row_y, fg, bg, buf);

        snprintf(buf, sizeof(buf), "%5.1f%%", current_procs[i].cpu_percent);
        ui_draw_text(18, row_y, ui_usage_color(current_procs[i].cpu_percent), bg, buf);

        snprintf(buf, sizeof(buf), "%8.1f MB", current_procs[i].mem_mb);
        ui_draw_text(25, row_y, ui_memory_color(current_procs[i].mem_mb), bg, buf);

        ui_draw_text(38, row_y, fg, bg, current_procs[i].command);
    }
}

void ui_draw_process_details(void) {
    int h = tb_height();
    int y = h - 4;

    if (y < 0)
        return;

    if (current_visible <= 0) {
        ui_draw_text(0, y, TB_CYAN | TB_BOLD, TB_DEFAULT, "Selected: ");
        ui_draw_text(10, y, TB_YELLOW, TB_DEFAULT, "none");
        return;
    }

    tb_printf(0, y, TB_CYAN | TB_BOLD, TB_DEFAULT, "Selected PID: ");
    tb_printf(14, y, TB_DEFAULT, TB_DEFAULT, "%d", current_procs[current_selected].pid);
    tb_printf(19, y, TB_MAGENTA | TB_BOLD, TB_DEFAULT, "USER: ");
    tb_printf(25, y, TB_DEFAULT, TB_DEFAULT, "%s", current_procs[current_selected].user);
    tb_printf(37, y, TB_CYAN | TB_BOLD, TB_DEFAULT, "CPU: ");
    tb_printf(42, y, ui_usage_color(current_procs[current_selected].cpu_percent), TB_DEFAULT, "%.1f%%",
              current_procs[current_selected].cpu_percent);
    tb_printf(50, y, TB_MAGENTA | TB_BOLD, TB_DEFAULT, "MEM: ");
    tb_printf(55, y, ui_memory_color(current_procs[current_selected].mem_mb), TB_DEFAULT, "%.1f MB",
              current_procs[current_selected].mem_mb);
    tb_printf(0, y + 1, TB_CYAN | TB_BOLD, TB_DEFAULT, "COMMAND: ");
    tb_printf(9, y + 1, TB_DEFAULT, TB_DEFAULT, "%s", current_procs[current_selected].command);
}

void ui_log_processes(FILE *f) {
    if (!f)
        return;

    fprintf(f, "PID USER CPU MEM COMMAND\n");

    for (int i = 0; i < current_visible; i++) {
        fprintf(f, "%d %s %.1f %.1f %s\n",
                current_procs[i].pid,
                current_procs[i].user,
                current_procs[i].cpu_percent,
                current_procs[i].mem_mb,
                current_procs[i].command);
    }

    fprintf(f, "\n");
    fflush(f);
}
