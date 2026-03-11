#include "system.h"

#include <stdio.h>
#include <string.h>

double get_cpu_usage(void) {
    static long prev_idle = 0;
    static long prev_total = 0;

    long user, nice, system, idle, iowait, irq, softirq;
    FILE *f = fopen("/proc/stat", "r");

    if (!f) return 0.0;

    fscanf(f, "cpu %ld %ld %ld %ld %ld %ld %ld",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq);

    fclose(f);

    long idle_now = idle + iowait;
    long total_now = user + nice + system + idle + iowait + irq + softirq;

    long idle_delta = idle_now - prev_idle;
    long total_delta = total_now - prev_total;

    prev_idle = idle_now;
    prev_total = total_now;

    if (total_delta == 0) return 0.0;

    return 100.0 * (1.0 - ((double)idle_delta / total_delta));
}

double get_memory_total() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;

    char line[256];
    double total = 0;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal: %lf kB", &total) == 1) {
            break;
        }
    }

    fclose(f);
    return total / (1024 * 1024);
}

double get_memory_used() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;

    char line[256];
    double total = 0;
    double available = 0;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal: %lf kB", &total) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %lf kB", &available) == 1)
            break;
    }

    fclose(f);

    double used = total - available;
    return used / (1024 * 1024);
}
