#ifndef SYSTEM_H
#define SYSTEM_H

#define MAX_CPU_CORES 128
#define MAX_GPUS 16

double get_cpu_usage(void);
int get_core_cpu_usage(double *usage, int max_cores);
int get_gpu_usage(double *usage, double *mem_used, double *mem_total, int max_gpus);
int get_load_average(double *load1, double *load5, double *load15);
double get_uptime(void);
double get_memory_used();
double get_memory_total();

#endif
