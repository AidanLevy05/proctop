#include "proc.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int proc_get_list(struct process *list) {
    DIR *dir = opendir("/proc");
    struct dirent *entry;

    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);

        list[count].pid = pid;

        char path[256];
        sprintf(path, "/proc/%d/comm", pid);

        FILE *f = fopen(path, "r");

        if (f) {
            fgets(list[count].command, 64, f);
            fclose(f);
        }

        count++;

        if (count >= MAX_PROCS)
            break;
    }

    closedir(dir);

    return count;
}

