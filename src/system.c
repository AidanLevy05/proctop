#include "system.h"

#include <stdio.h>

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

