#include "ui.h"
#include "proc.h"
#include "system.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PROCTOP_VERSION "0.1.0"

void set_timed_status(const char *message, int *status_ticks) {
    ui_set_status_message(message);
    *status_ticks = 2;
}

void update_search_status(const char *search) {
    char message[128];

    snprintf(message, sizeof(message), "Search: %s", search);
    ui_set_status_message(message);
}

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
    printf("  t            Toggle tree mode.\n");
    printf("  /            Search/filter interactively.\n");
    printf("  K            Terminate selected process.\n");
    printf("  X            Force kill selected process.\n");
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
    int confirm_kill = 0;
    int confirm_signal = SIGTERM;
    int kill_pid = -1;
    int search_mode = 0;
    int search_len = 0;
    int status_ticks = 0;
    int tree_mode = 0;
    char *log_path = NULL;
    char *filter = NULL;
    FILE *log_file = NULL;
    char search[64] = "";

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
            double uptime = get_uptime();
            double core_usage[MAX_CPU_CORES];
            int core_count = get_core_cpu_usage(core_usage, MAX_CPU_CORES);
            int gpu_count = get_gpu_usage(gpu_usage, gpu_mem_used, gpu_mem_total, MAX_GPUS);

            get_load_average(&load1, &load5, &load15);

            ui_clear();

            ui_draw_header();
            ui_draw_system(cpu_usage, mem_used, mem_total, load1, load5, load15, uptime);
            ui_draw_gpu(gpu_usage, gpu_mem_used, gpu_mem_total, gpu_count);
            ui_draw_core_cpu(core_usage, core_count);
            ui_draw_process_table();
            ui_draw_process_details();
            ui_draw_status();

            ui_present();

            if (log_file)
                log_snapshot(log_file);

            if (status_ticks > 0) {
                status_ticks--;

                if (status_ticks == 0)
                    ui_clear_status_message();
            }

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
            if (search_mode) {
                if (ev.key == TB_KEY_ESC) {
                    search_mode = 0;
                    set_timed_status("Search canceled", &status_ticks);
                    refresh = 1;
                    continue;
                }

                if (ev.key == TB_KEY_ENTER) {
                    search_mode = 0;
                    proc_set_filter(search);

                    if (search[0] != '\0')
                        set_timed_status("Search applied", &status_ticks);
                    else
                        set_timed_status("Search cleared", &status_ticks);

                    refresh = 1;
                    continue;
                }

                if (ev.key == TB_KEY_BACKSPACE || ev.key == TB_KEY_BACKSPACE2) {
                    if (search_len > 0) {
                        search_len--;
                        search[search_len] = '\0';
                    }

                    update_search_status(search);
                    refresh = 1;
                    continue;
                }

                if (ev.ch >= 32 && ev.ch < 127 && search_len < (int)sizeof(search) - 1) {
                    search[search_len] = ev.ch;
                    search_len++;
                    search[search_len] = '\0';
                    update_search_status(search);
                    refresh = 1;
                    continue;
                }

                refresh = 1;
                continue;
            }

            if (confirm_kill) {
                if ((ev.ch == 'Y' || ev.ch == 'y') && kill_pid > 0) {
                    if (kill(kill_pid, confirm_signal) != 0) {
                        char message[128];

                        if (confirm_signal == SIGKILL)
                            snprintf(message, sizeof(message), "Failed to kill PID %d", kill_pid);
                        else
                            snprintf(message, sizeof(message), "Failed to terminate PID %d", kill_pid);

                        set_timed_status(message, &status_ticks);
                    } else {
                        char message[128];

                        if (confirm_signal == SIGKILL)
                            snprintf(message, sizeof(message), "Sent SIGKILL to PID %d", kill_pid);
                        else
                            snprintf(message, sizeof(message), "Sent SIGTERM to PID %d", kill_pid);

                        set_timed_status(message, &status_ticks);
                    }
                } else {
                    if (confirm_signal == SIGKILL)
                        set_timed_status("Force kill canceled", &status_ticks);
                    else
                        set_timed_status("Terminate canceled", &status_ticks);
                }

                confirm_kill = 0;
                confirm_signal = SIGTERM;
                kill_pid = -1;
                refresh = 1;
                continue;
            }

            if (ev.ch == 'q' || ev.key == TB_KEY_ESC) {
                break;
            }
            if (ev.ch == 'r') {
                ui_clear_status_message();
                status_ticks = 0;
                refresh = 1;
                continue;
            }
            if (ev.ch == 'm') {
                ui_clear_status_message();
                status_ticks = 0;
                proc_set_sort_mode(PROC_SORT_MEM);
                refresh = 1;
                continue;
            }
            if (ev.ch == 'p') {
                ui_clear_status_message();
                status_ticks = 0;
                proc_set_sort_mode(PROC_SORT_PID);
                refresh = 1;
                continue;
            }
            if (ev.ch == 'c') {
                ui_clear_status_message();
                status_ticks = 0;
                proc_set_sort_mode(PROC_SORT_CPU);
                refresh = 1;
                continue;
            }
            if (ev.ch == 't') {
                tree_mode = tree_mode ? 0 : 1;
                proc_set_tree_mode(tree_mode);

                if (tree_mode)
                    set_timed_status("Tree mode enabled", &status_ticks);
                else
                    set_timed_status("Tree mode disabled", &status_ticks);

                refresh = 1;
                continue;
            }
            if (ev.ch == '/') {
                search_mode = 1;
                search_len = 0;
                search[0] = '\0';
                status_ticks = 0;
                update_search_status(search);
                refresh = 1;
                continue;
            }
            if (ev.ch == 'K') {
                kill_pid = ui_get_selected_pid();

                if (kill_pid > 0) {
                    char message[128];

                    confirm_kill = 1;
                    confirm_signal = SIGTERM;
                    status_ticks = 0;
                    snprintf(message, sizeof(message), "Terminate PID %d? (Y/N)", kill_pid);
                    ui_set_status_message(message);
                } else {
                    set_timed_status("No process selected", &status_ticks);
                }

                refresh = 1;
                continue;
            }
            if (ev.ch == 'X') {
                kill_pid = ui_get_selected_pid();

                if (kill_pid > 0) {
                    char message[128];

                    confirm_kill = 1;
                    confirm_signal = SIGKILL;
                    status_ticks = 0;
                    snprintf(message, sizeof(message), "Force kill PID %d? (Y/N)", kill_pid);
                    ui_set_status_message(message);
                } else {
                    set_timed_status("No process selected", &status_ticks);
                }

                refresh = 1;
                continue;
            }
            if (ev.key == TB_KEY_ARROW_UP) {
                ui_clear_status_message();
                status_ticks = 0;
                ui_move_selection(-1);
                refresh = 1;
                continue;
            }
            if (ev.key == TB_KEY_ARROW_DOWN) {
                ui_clear_status_message();
                status_ticks = 0;
                ui_move_selection(1);
                refresh = 1;
                continue;
            }

            ui_clear_status_message();
            status_ticks = 0;
        }

        refresh = 1;
    }

    ui_shutdown();

    if (log_file)
        fclose(log_file);

    return 0;
}
