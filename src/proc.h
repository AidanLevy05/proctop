#ifndef PROC_H
#define PROC_H

#define MAX_PROCS 4096
#define PROC_SORT_MEM 0
#define PROC_SORT_PID 1
#define PROC_SORT_CPU 2
#define PROC_DISPLAY_LIMIT 25

struct process {
    int pid;
    int ppid;
    int threads;
    int tree_level;
    char command[64];
    char user[32];
    char state;
    double mem_mb;
    double cpu_percent;
};

void proc_set_sort_mode(int mode);
void proc_set_tree_mode(int enabled);
void proc_set_filter(const char *filter);
int proc_get_list(struct process *list);

#endif
