#ifndef PROC_H
#define PROC_H

#define MAX_PROCS 4096

struct process {
    int pid;
    char command[64];
    char user[32];
    double mem_mb;
};

int proc_get_list(struct process *list);

#endif
