#include "ui.h"
#include "proc.h"
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PROCTOP_VERSION "0.1.0"

void print_help(void) {
    printf("Usage: proctop [--log file] [filter]\n");
    printf("\n");
    printf("filter:\n");
    printf("  Show only processes whose command contains the given substring.\n");
    printf("\n");
    printf("Options:\n");
    printf("  --help       Show this help text.\n");
    printf("  --version    Show version information.\n");
    printf("  --log file   Append process snapshots to a text file.\n");
    printf("\n");
    printf("Controls:\n");
    printf("  q / Esc      Quit.\n");
    printf("  r            Refresh.\n");
    printf("  m            Sort by memory.\n");
    printf("  p            Sort by pid.\n");
    printf("  c            Sort by cpu.\n");
    printf("  Up/Down      Move selected process.\n");
}

void print_version(void) {
    printf("proctop %s\n", PROCTOP_VERSION);
}

void log_snapshot(FILE *log_file) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];

    if (tm_info && strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info) > 0)
        fprintf(log_file, "[%s]\n", timestamp);
    else
        fprintf(log_file, "[%ld]\n", (long)now);

    ui_log_processes(log_file);
}

int main(int argc, char **argv) {
    struct tb_event ev;
    int refresh = 1;
    char *log_path = NULL;
    char *filter = NULL;
    FILE *log_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        }

        if (strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        }

        if (strcmp(argv[i], "--log") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing file after --log\n");
                return 1;
            }

            log_path = argv[++i];
            continue;
        }

        if (argv[i][0] == '-' && argv[i][1] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!filter)
            filter = argv[i];
        else {
            fprintf(stderr, "Too many arguments\n");
            return 1;
        }
    }

    if (filter)
        proc_set_filter(filter);

    if (log_path) {
        log_file = fopen(log_path, "a");

        if (!log_file) {
            perror(log_path);
            return 1;
        }
    }

    ui_init();

    while (1) {
        if (refresh)
        {
            double cpu_usage = get_cpu_usage();
            double mem_used = get_memory_used();
            double mem_total = get_memory_total();
            double gpu_usage[MAX_GPUS];
            double gpu_mem_used[MAX_GPUS];
            double gpu_mem_total[MAX_GPUS];
            double load1 = 0.0;
            double load5 = 0.0;
            double load15 = 0.0;
            double core_usage[MAX_CPU_CORES];
            int core_count = get_core_cpu_usage(core_usage, MAX_CPU_CORES);
            int gpu_count = get_gpu_usage(gpu_usage, gpu_mem_used, gpu_mem_total, MAX_GPUS);

            get_load_average(&load1, &load5, &load15);

            ui_clear();

            ui_draw_header();
            ui_draw_system(cpu_usage, mem_used, mem_total, load1, load5, load15);
            ui_draw_gpu(gpu_usage, gpu_mem_used, gpu_mem_total, gpu_count);
            ui_draw_core_cpu(core_usage, core_count);
            ui_draw_process_table();
            ui_draw_process_details();
            ui_draw_status();

            ui_present();

            if (log_file)
                log_snapshot(log_file);

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
            if (ev.key == TB_KEY_ARROW_UP) {
                ui_move_selection(-1);
                refresh = 1;
                continue;
            }
            if (ev.key == TB_KEY_ARROW_DOWN) {
                ui_move_selection(1);
                refresh = 1;
                continue;
            }
        }

        refresh = 1;
    }

    ui_shutdown();

    if (log_file)
        fclose(log_file);

    return 0;
}
