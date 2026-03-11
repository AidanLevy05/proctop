#define _POSIX_C_SOURCE 200809L

#include "system.h"

#include <stdio.h>
#include <string.h>

double get_cpu_usage(void) {
    static long long prev_idle = 0;
    static long long prev_total = 0;

    long long user, nice, system, idle, iowait, irq, softirq, steal;
    FILE *f = fopen("/proc/stat", "r");

    if (!f) return 0.0;

    if (fscanf(f, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) != 8)
    {
        fclose(f);
        return 0.0;
    }

    fclose(f);

    long long idle_now = idle + iowait;
    long long total_now = user + nice + system + idle + iowait + irq + softirq + steal;

    long long idle_delta = idle_now - prev_idle;
    long long total_delta = total_now - prev_total;
    int initialized = prev_total != 0;

    prev_idle = idle_now;
    prev_total = total_now;

    if (!initialized || total_delta == 0) return 0.0;

    return 100.0 * (1.0 - ((double)idle_delta / total_delta));
}

int get_core_cpu_usage(double *usage, int max_cores) {
    static long long prev_idle[MAX_CPU_CORES];
    static long long prev_total[MAX_CPU_CORES];
    FILE *f = fopen("/proc/stat", "r");
    char line[256];
    int count = 0;

    if (!f)
        return 0;

    while (fgets(line, sizeof(line), f)) {
        int cpu;
        long long user, nice, system, idle, iowait, irq, softirq, steal;

        if (count >= max_cores || count >= MAX_CPU_CORES)
            break;

        if (sscanf(line, "cpu%d %lld %lld %lld %lld %lld %lld %lld %lld",
                   &cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) == 9)
        {
            long long idle_now = idle + iowait;
            long long total_now = user + nice + system + idle + iowait + irq + softirq + steal;
            long long idle_delta = idle_now - prev_idle[count];
            long long total_delta = total_now - prev_total[count];
            int initialized = prev_total[count] != 0;

            prev_idle[count] = idle_now;
            prev_total[count] = total_now;

            if (!initialized || total_delta <= 0)
                usage[count] = 0.0;
            else
                usage[count] = 100.0 * (1.0 - ((double)idle_delta / total_delta));

            count++;
        }
    }

    fclose(f);
    return count;
}

int get_gpu_usage(double *usage, double *mem_used, double *mem_total, int max_gpus) {
    FILE *f = popen("nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total --format=csv,noheader,nounits 2>/dev/null", "r");
    char line[256];
    int count = 0;

    if (!f)
        return 0;

    while (fgets(line, sizeof(line), f)) {
        double gpu_usage, gpu_mem_used, gpu_mem_total;

        if (count >= max_gpus || count >= MAX_GPUS)
            break;

        if (sscanf(line, " %lf , %lf , %lf", &gpu_usage, &gpu_mem_used, &gpu_mem_total) == 3) {
            usage[count] = gpu_usage;
            mem_used[count] = gpu_mem_used / 1024.0;
            mem_total[count] = gpu_mem_total / 1024.0;
            count++;
        }
    }

    pclose(f);
    return count;
}

int get_load_average(double *load1, double *load5, double *load15) {
    FILE *f = fopen("/proc/loadavg", "r");

    if (!f)
        return 0;

    if (fscanf(f, "%lf %lf %lf", load1, load5, load15) != 3) {
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

double get_uptime(void) {
    FILE *f = fopen("/proc/uptime", "r");
    double uptime = 0.0;

    if (!f)
        return 0.0;

    if (fscanf(f, "%lf", &uptime) != 1)
        uptime = 0.0;

    fclose(f);
    return uptime;
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
